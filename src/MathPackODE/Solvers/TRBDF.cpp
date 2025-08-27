#include "TRBDF.h"

namespace Solvers{
    void TRBDF::makeTRJ(vector<vector<double>> & J){
        for(int i=0; i<diffRank; i++){
            for(int j=0; j<diffRank; j++){
                double add = i==j ? 1.0 : 0.0;
                J[i][j] = add-gamma*dt/2.0*J[i][j];
            }
        }
    }
    void TRBDF::makeBDFJ(vector<vector<double>> & J){
        for(int i=0; i<diffRank; i++){
            for(int j=0; j<diffRank; j++){
                double add = i==j ? 1.0 : 0.0;
                J[i][j] = add-(1.0-gamma)/(2.0-gamma)*dt*J[i][j];
            }
        }
    }

    /**
     * fng=dXvector
     * xng=Xvector
     */
    void TRBDF::trF(){
        for (int i = 0; i < diffRank; i++) {
            fng[i] = *dXvector[i];
            newtonF[i] = (*Xvector[i]-xn[i])-gamma*dt/2.0*(fng[i]+fn[i]);
        }
    }

    /**
     * x_(n+1)=Xvector
     */
    void TRBDF::bdfF(){
        for (int i = 0; i < diffRank; i++) {
            fnn[i] = *dXvector[i];
            newtonF[i] = (*Xvector[i]-xng[i]/(gamma*(2.0-gamma))+xn[i]*(1.0-gamma)*(1.0-gamma)/gamma/(2.0-gamma))
                    -(1.0-gamma)/(2.0-gamma)*dt*fnn[i];
        }
    }

    void TRBDF::selfInit() {
        xn = vector<double>(diffRank, 0);
        xng = vector<double>(diffRank, 0);
        copyArray(Xvector,xn);

        fn = vector<double>(diffRank, 0);
        copyArray(dXvector,fn);
        fng = vector<double>(diffRank, 0);
        fnn = vector<double>(diffRank, 0);

        newtonF = vector<double>(diffRank, 0);

        dF = vector<vector<double>>(diffRank, vector<double>(diffRank,0));
    }

    

    /**
     * Estimates d(d.X)/dX
     * @param J - output
     * @param fx - target function
     * @param f0 - function value at fx(x0)
     * @param x - x0 initial point
     * @param fac
     */
    void TRBDF::estimJ(vector<vector<double>> & J,
                vector<shared_ptr<MathPack::StringGraph>> const& fx,
                vector<double> const& f0,
                vector<double*> & x,
                vector<double> & fac) {
        size_t ny=f0.size();
        for(size_t i=0; i<ny; i++){
            double x0 = *x[i],
                    del = x0*1e-3;
            del = del==0 ? 1e-6 : del;
            *x[i] = x0+del;

            evalSysState();

            for(size_t j=0; j<ny; j++){
                J[j][i]=(*dXvector[j]-f0[j])/del;
            }

            *x[i] = x0;
        }
    }

    void TRBDF::evalNextStep() {
        vector<double> dummy;
        double t0=time->getValue();
        time->setValue(t0+gamma*dt);
        //  TR step
        estimJ(dF, {}, fn, Xvector, dummy); // estimate d(d.X)/dX
        makeTRJ(dF);

        while(true){
            trF();
            double err = MathPack::MatrixEqu::norm(newtonF);
            while (err > tolerance) {
                MathPack::MatrixEqu::solveLU(dF, newtonF); //  dF*deltaX=f0
                for (int i = 0; i < diffRank; i++) {
                    double val = *Xvector[i] - newtonF[i];
                    *Xvector[i] = val;
                    xng[i] = val;
                }

                evalSysState();
                trF(); // eval F(X)

                err = MathPack::MatrixEqu::norm(newtonF);
                if (err > tolerance) {
                    //update J
                    estimJ(dF, {}, fng, Xvector, dummy); // estimate d(d.X)/dX
                    makeTRJ(dF);
                } else {
                    break;
                }
            }

            time->setValue(t0+dt);
            // BDF2 step
            estimJ(dF, {}, fng, Xvector, dummy); // estimate d(d.X)/dX
            makeBDFJ(dF);
            bdfF();
            err = MathPack::MatrixEqu::norm(newtonF);
            while (err > tolerance) {
                MathPack::MatrixEqu::solveLU(dF, newtonF); //  dF*deltaX=f0
                for (int i = 0; i < diffRank; i++) {
                    *Xvector[i] = *Xvector[i] - newtonF[i];
                }

                evalSysState();
                bdfF(); // eval F(X)

                err = MathPack::MatrixEqu::norm(newtonF);
                if (err > tolerance) {
                    //update J
                    estimJ(dF, {}, fng, Xvector, dummy); // estimate d(d.X)/dX
                    makeBDFJ(dF);
                } else {
                    break;
                }
            }

            //err
            err = 0;
            double norm = 0;
            for (int i = 0; i < diffRank; i++) {
                norm += xn[i] * xn[i];
                double v = 2.0 * kGamma * dt * (1.0 / gamma * fn[i] - fng[i] / gamma / (1.0 - gamma) + fnn[i] / (1.0 - gamma));
                err += v * v;
            }
            norm = sqrt(norm);
            err = sqrt(err);
            double r = err / (relTol * norm + 1e-6);
            if (r > 2) {
                dt /= 10.0;
                time->setValue(t0);

                if(dt<hmin){
                    throw runtime_error("Stepsize too small: "+to_string(dt)+", at t="+to_string(t0));
                }
                for(int i=0;i<diffRank;i++){
                    *Xvector[i] = xn[i];
                }
                //evalSysState();

                //update J
                estimJ(dF, {}, fn, Xvector, dummy); // estimate d(d.X)/dX
                makeBDFJ(dF);

            } else {
                double temp = 1.25*pow(err/relTol,1.0/3.0);
                if (temp > 0.2)
                    dt = dt / temp;
                else
                    dt = 5.0*dt;
                dt=min(dt,hmax);
                break;
            }

        }
        copyArray(Xvector,xn);
        memcpy(fn.data(), fnn.data(), diffRank*sizeof(double));
    }
}
