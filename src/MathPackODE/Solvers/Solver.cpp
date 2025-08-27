#include "Solver.h"

using namespace std;
using namespace dae_solver;

namespace Solvers{

    void Solver::preEvaluate(){
        // for(Updatable el:dae.getUpdatableElements())
        //     el.preEvaluate(time.getValue());
    }

    void Solver::postEvaluate(){
        dae->updateElemWorkspaces();
    }

    void Solver::updateOutputs() {
        discSys->discreteStep(time->getValue());
    }
    
    void Solver::evalMathDX(){
        discSys->continuousStep();
    }

    void Solver::selfInit(){}

    void Solver::copyArray(vector<double*> const& source, vector<double> & destination){
        for (int i=0;i<source.size();i++) {
            destination[i] = *source[i];
        }
    }

    /**
     * Evaluates estimation of Jacobian of fx near f0=fx(x0) point.
     * @param J - output
     * @param fx - target function
     * @param f0 - function value at fx(x0)
     * @param x - x0 initial point
     * @param fac
     */
    void Solver::estimJ(vector<vector<double>> & J,
                    vector<shared_ptr<MathPack::StringGraph>> const& fx,
                    vector<double> const& f0,
                    vector<double*> & x,
                    vector<double> & fac)
    {
        double eps=nextafter(1.0, 2.0) - 1.0,
                br=pow(eps,0.875),
                bl=pow(eps,0.75),
                bu=pow(eps,0.25),
                facmin=pow(eps,0.98),
                facmax=1e15;
        int ny=fx.size();
        vector<double> f1(ny),
                x0(ny);
        for(int i=0; i<ny; i++){
            x0[i] = *x[i];
        }

        // row cycle
        int i=0;
        while(i<ny){

            double yScale=max(abs(x0[i]),0.0); //?
            yScale=yScale==0.0 ? 160*eps : yScale;

            double del=fac[i]*abs(yScale);
            *x[i] = x0[i] + del;  //shift X

//            evalSysState();

            for(int m=0;m<ny;m++) {
                f1[m] = fx.at(m)->evaluate();
            }
            double maxDiff=-1.0;
            int max=-1;

            for(int j=0;j<ny;j++){
//                f1[j]=dXvector.get(j).getValue();

                double aVal=abs(f1[j]-f0[j]);
                if(aVal>maxDiff) {
                    maxDiff=aVal;
                    max = j;
                }
            }

            double scale = std::max(abs(f1[max]), abs(f0[max]));

            if(min(abs(f1[max]),abs(f0[max]))==0) {
                for(int j=0;j<ny;j++){
                    J[j][i]=(f1[j]-f0[j])/del;
                }
                *x[i] = x0[i];
                i++;
            }else if(maxDiff>bu*scale){
                fac[i]=std::max(0.013*fac[i],facmin);
            }else if(maxDiff<=bl*scale){
                fac[i]=min(11*fac[i],facmax);
            }else if(maxDiff<br*scale){
                cerr << "bad!";
            }else{
                for(int j=0;j<ny;j++){
                    J[j][i]=(f1[j]-f0[j])/del;
                }
                *x[i] = x0[i];
                i++;
            }
        }
    }

    void Solver::updateJ(vector<vector<double>> & J, vector<double> const& fNew, vector<double> const& fOld, vector<double> const& xDelta){
        double denum=0,num=0;
        int len=J.size();
        vector<vector<double>> tmp=J;
        vector<double> rowTmp(len);
        for(int i=0;i<len;i++){
            for(int j=0;j<len;j++){
                num+=J[i][j]*(xDelta[j]);
            }
            rowTmp[i]=num;
            num=0;

            denum+=(xDelta[i])*(xDelta[i]);
        }
        for(int i=0;i<len;i++){
            rowTmp[i]=fNew[i]-fOld[i]-rowTmp[i];
        }
        for(int i=0;i<len;i++) {
            for (int j = 0; j < len; j++) {
                J[i][j]+=rowTmp[i]*(xDelta[j])/denum;
            }
        }
    }

    Solver::Solver(){}

    void Solver::init(Compiler & compiler,
                    ModelState *state,
                    SolverWorker *tsk,
                    shared_ptr<dae_solver::Logger> logger)
    {
        this->logger = logger;
        workerThread=tsk;
        dae = compiler.DAEsys;
        vars = compiler.workspace;

        QObject::connect(workerThread, &SolverWorker::cancelled, [this](){cancelFlag=true;});

        time = vars->get(WorkSpace::TIME_VAR_NAME);
        tEnd = state->getTend();
        progress = 0.0;
        

        symbJacobian = compiler.DAEsys->getJacob();
        invJacob = compiler.DAEsys->getInvJacob();
        newtonFunc = compiler.DAEsys->getNewtonFunc();
        algSystem = compiler.DAEsys->getAlgSystem();
        jacobEstType = state->getJacobianEstimationType();

        dt = state->getDt();
        absTol = state->getAbsTol();
        relTol = state->getRelTol();

        vals= vector<double>(algSystem.size());
        J = vector<vector<double>>(algSystem.size(), vector<double>(algSystem.size(), 0));

        Xvector = dae->xVector;
        dXvector = dae->dXVector;
        stateVector = dae->stateVector;
        // dynamic discrete
        discSys = compiler.discSys;
        Xvector.insert(Xvector.end(), discSys->Xvector.begin(), discSys->Xvector.end());
        dXvector.insert(dXvector.end(), discSys->dXvector.begin(), discSys->dXvector.end());

        // verify state size
        if (stateVector.size() != algSystem.size()){
            ostringstream oss;
            oss << "Incorrect system! State has size:"<<stateVector.size()<<endl;
            oss << "But algebraic system has "<<algSystem.size()<< " equations.";
            throw runtime_error(oss.str());
        }

        // zerotime init
        // TODO reset dynamic vars!
        evalSysState();

        // ???
        testFac = vector<double>(stateVector.size(), 1e15);
        testFlag=true;

        
        discSys->init();

        postEvaluate();
        updateOutputs();

        selfInit();
    }

    void Solver::solve(){
        //for(time=dt;time<=tEnd;time=time+dt){
        while(time->getValue() < tEnd){
            if(cancelFlag)
                break;
            preEvaluate();
            evalNextStep();
            postEvaluate();
            updateOutputs();
            progress = time->getValue()/tEnd*100.0;
        }
    }

    void Solver::evalSysState(){
        int cnt=0;
        double tol=1e-6,
                m=1.0;

        if(!symbJacobian.empty()) {
            bool faultflag = false;

            while (true) {
                if (cancelFlag)
                    break;

                //eval F(x)=0
                MathPack::MatrixEqu::putValuesFromSymbRow(vals, algSystem);
                double norm = MathPack::MatrixEqu::norm(vals);

                switch (jacobEstType) {
                    case 0:{
                        //full symbolic
//                            MathPack.MatrixEqu.putValuesFromSymbRow(vals, newtonFunc, vars);

                        //test estimJ
                        if(testFlag){
                            MathPack::MatrixEqu::putValuesFromSymbMatr(J, symbJacobian);

                            testFlag=false;
                        }else {
                            estimJ(J, algSystem, vals, stateVector, testFac);
                            auto jj = vector<vector<double>>(J.size(), vector<double>(J.size()));
                            MathPack::MatrixEqu::putValuesFromSymbMatr(jj, symbJacobian);
                            
                            // debug
                            if(false)
                                for (int k = 0; k < J.size(); k++) {
                                    for (int q = 0; q < J.size(); q++) {
                                        cout << format("({}/{})  ", jj[k][q], J[k][q]);
                                    }
                                    cout << endl;
                                }
                        }
                        MathPack::MatrixEqu::solveLU(J, vals); // first 'vals' contains F(x)
                        break;
                    }
                    case 1: {
                        //inverse symbolic
                        auto invJ = MathPack::MatrixEqu::evalSymbMatr(invJacob);
                        // Note: allocates memory!
                        vals = MathPack::MatrixEqu::mulMatxToRow(invJ, vals);
                        break;
                    }
                    case 2: {
                        //only jacob symb
                        MathPack::MatrixEqu::putValuesFromSymbMatr(J, symbJacobian);
                        MathPack::MatrixEqu::solveLU(J, vals); // first 'vals' contains F(x)
                        break;
                    }
                    default:
                        vals.clear();
                }

                for (int i = 0; i < vals.size(); i++) {
                    if (isnan(vals[i])) {
                        throw runtime_error(format("X{} is not a number! At time {}", i, time->getValue()));
                    } else if (!isfinite(vals[i])) {
                        throw runtime_error(format("X{} is not finite! At time {}", i, time->getValue()));
                    }

                    double newval = (*stateVector[i]) - m*vals[i];
                    *stateVector[i] = newval;
                }

                MathPack::MatrixEqu::putValuesFromSymbRow(vals, algSystem);
                norm = MathPack::MatrixEqu::norm(vals);
                if (norm < tol)
                    break;

                cnt++;
                if (cnt > 250) {
                    if (false) { // for debug
                        for (auto& row : J) {
                            for(auto& v:row)
                                logger->printErr(to_string(v) + " ");
                            logger->printErr("\n");
                        }
                        logger->printErr("x: ");
                        for (auto& v:vals)
                            logger->printErr(to_string(v) + " ");
                        logger->printErr("\nF(x): ");
                        MathPack::MatrixEqu::putValuesFromSymbRow(vals, algSystem);
                        for (auto& v:vals)
                            logger->printErr(to_string(v) + " ");
                        logger->printErr("\n");
                    }
                    //tol*=10;
                    cnt = 0;
                    m=0.5;
                    if (faultflag) {
                        throw runtime_error("Dead loop! At time "+to_string(time->getValue()));
                    } else {

                        faultflag = true;
//                            int ii = 0;
//                            for (String var : vars.getVarNameList()) {
//
//                                if (!var.startsWith("X.")) {
//                                    vars.setValue(var, 0);
//                                    x0[ii] = 0;
//                                    //s[ii]=0;
//                                    ii++;
//                                }
//                            }
                    }
                }
            }
            
        }
        evalMathDX();

        // for debug
        #ifdef DEBUG
            if (time->getValue() == 0) {
                logger->printMsg("\n\nt ");
                for (size_t i=0; i<stateVector.size();i++) {
                    logger->printMsg(format("S{} ", i));
                }
                for (size_t i=0; i<Xvector.size();i++) {
                    logger->printMsg(format("X{} ", i));
                }
                for (size_t i=0; i<dXvector.size();i++) {
                    logger->printMsg(format("dX{} ", i));
                }
                logger->printMsg(" numOfJac\n");
            }
            logger->printMsg(to_string(time->getValue()) + " ");
            for (auto& entry : stateVector) {
                logger->printMsg(to_string(*entry) + " ");
            }
            for (auto& entry : Xvector) {
                logger->printMsg(to_string(*entry) + " ");
            }
            for (auto& entry : dXvector) {
                logger->printMsg(to_string(*entry) + " ");
            }
            logger->printMsg(to_string(cnt) + "\n");
        #endif
    }
    
}
