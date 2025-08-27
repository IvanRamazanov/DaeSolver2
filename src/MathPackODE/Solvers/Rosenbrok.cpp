#include "Rosenbrok.h"

namespace Solvers{
    /**
     * x0 must be set
     */
    void Rosenbrok::estimT(){
        //dfdt=(f1-f0)/tdel
        //    tdel = (t + min(sqrt(eps)*(t+dt),dt)) - t;
        double t0 = time->getValue(),
                tdel = (t0 + min(sqrtEps*(t0+dt), dt)) - t0;

        time->setValue(t0+tdel); // frankly, must be +=tdel
        evalSysState();

        for(int i=0;i<diffRank;i++) {
            F1[i] = *dXvector[i];
            dfdt[i] = (F1[i]-F0[i])/tdel;
        }
        time->setValue(t0);
    }

    double Rosenbrok::error(){
        double normy = MathPack::MatrixEqu::norm(x0),
        newNorm = MathPack::MatrixEqu::norm(xnew);

        // eval norm of k1 - 2.0 * k2 + k3
        double sum=0.0;
        for(int i=0;i<diffRank;i++) {
            sum+=(k1[i] - 2.0 * k2[i] + k3[i])*(k1[i] - 2.0 * k2[i] + k3[i]);
        }
        sum=sqrt(sum);


        sum = (dt / 6.0) * (sum  / max(max(normy, newNorm), 1e-6));

        return sum;
    }

    void Rosenbrok::evalW(){
        for(int i=0;i<diffRank;i++){
            for(int j=0;j<diffRank;j++){
                W[i][j]=-1.0*dt*d*dfdx[i][j];

                if(i==j)
                    W[i][j]+=1.0;
            }
        }
    }

    void Rosenbrok::selfInit() {
        dfdt = vector<double>(diffRank, 0);
        dfdx = vector<vector<double>>(diffRank, vector<double>(diffRank, 0));
        W = vector<vector<double>>(diffRank, vector<double>(diffRank, 0));
        x0 = vector<double>(diffRank, 0);
        xnew = vector<double>(diffRank, 0);
        F0 = vector<double>(diffRank, 0);
        F1 = vector<double>(diffRank, 0);
        F2 = vector<double>(diffRank, 0);
        k1 = vector<double>(diffRank, 0);
        k2 = vector<double>(diffRank, 0);
        k3 = vector<double>(diffRank, 0);
        fac = vector<double>(diffRank, 0);

        for(int i=0; i<diffRank; i++) {
//            fac[i]=sqrtEps;
            fac[i] = 1e10;
        }
        for(int j=0;j<diffRank;j++) {
            F0[j] = *dXvector[j];
            x0[j] = *Xvector[j];
        }

        nSteps=0;
        nFailed=0;
    }

    /**
     * Evaluates estimation of Jacobian of fx near f0=fx(x0) point.
     * @param J - output
     * @param fx - target function
     * @param f0 - function value at fx(x0)
     * @param vars - x0 initial point
     * @param fac
     */
    void Rosenbrok::estimJ(vector<vector<double>> & J,
                vector<shared_ptr<MathPack::StringGraph>> const& fx,
                vector<double> const& f0,
                vector<double*> & x,
                vector<double> & fac) {
        const double eps = 2.220446049250313E-16,
                br = pow(eps, 0.875),
                bl = pow(eps, 0.75),
                bu = pow(eps, 0.25),
                facmin = pow(eps, 0.98) * 1e-10,
                facmax = 1e15;
        int ny = diffRank;
        vector<double> f1 = vector<double>(ny),
                        x0 = vector<double>(ny);
        for(int i=0; i<ny; i++){
            x0[i] = *x[i];
        }

        // row cycle
        int i=0;
        while(i<ny){
            double yScale = max(abs(x0[i]), 0.0); //?
            yScale = yScale==0.0 ? 160*eps : yScale;

//            double del=fac[i]*abs(yScale);
            double del = x0[i]==0.0 ? 1e-5 : x0[i]*1e-4;
            *x[i] = x0[i]+del;  //shift X

            evalSysState();

            for(int m=0; m<ny; m++) {
                f1[m] = *dXvector[m];
            }
            double maxDiff=-1.0;
            int max=-1;

            for(int j=0;j<ny;j++){
//                f1[j]=dXvector.get(j).getValue();

                double aVal = abs(f1[j]-f0[j]);
                if(aVal>maxDiff) {
                    maxDiff=aVal;
                    max = j;
                }
            }

            double scale = std::max(abs(f1[max]), abs(f0[max]));

            bool confirm=false;
            if(min(abs(f1[max]), abs(f0[max]))==0) {
                confirm=true;
            }else if(maxDiff > bu*scale){
                fac[i] = std::max(0.013*fac[i], facmin);
                if(fac[i] == facmin)
                    confirm=true;
            }else if(maxDiff <= bl*scale){
                fac[i] = min(11*fac[i],facmax);
                if(fac[i] == facmax)
                    confirm=true;
            }else if(maxDiff < br*scale){
                cerr << "bad!";
            }else{
                confirm=true;
            }
            if(confirm) {
                for (int j = 0; j < ny; j++) {
                    J[j][i] = (f1[j] - f0[j]) / del;
                }
                *x[i] = x0[i];
                i++;
            }
        }
    }

    Rosenbrok::Rosenbrok(){}

    void Rosenbrok::evalNextStep() {
        double t0 = time->getValue();

        // F0 exist
        estimJ(dfdx,algSystem,F0,Xvector,fac); //TODO not correct estimJ!
        evalW();

        // prepare k1 to use as an output
        estimT(); //estimate dfdt

        bool noFailed=true;
        double error;
        while(true) {

            for (int i = 0; i < diffRank; i++) {
                k1[i] = F0[i] + dt * d * dfdt[i];
            }

            //eval k1
            MathPack::MatrixEqu::solveLU(W, k1);

            //eval F1=F(t+0.5*dt, x+0.5*dt*k1)
            time->setValue(t0 + 0.5 * dt);
            for (int i = 0; i < diffRank; i++)
                *Xvector[i] = x0[i] + 0.5 * dt * k1[i];
            evalSysState();
            for (int i = 0; i < diffRank; i++)
                F1[i] = *dXvector[i];

            // prepare k2
            for (int i = 0; i < diffRank; i++)
                k2[i] = F1[i] - k1[i];
            //eval k2
            MathPack::MatrixEqu::solveLU(W, k2);
            for (int i = 0; i < diffRank; i++)
                k2[i] += k1[i];

            // eval x_(n+1)


            //eval F2=F(t_(n+1), x_(n+1))
            for (int i = 0; i < diffRank; i++) {
                double val = x0[i] + dt * k2[i];
                *Xvector[i] = val;
                xnew[i]=val;
            }
            time->setValue(t0 + dt);
            evalSysState();
            for (int i = 0; i < diffRank; i++)
                F2[i] = *dXvector[i];

            //eval k3
            for (int i = 0; i < diffRank; i++)
                k3[i] = F2[i] - e32 * (k2[i] - F1[i]) - 2.0 * (k1[i] - F0[i]) + dt * d * dfdt[i];
            MathPack::MatrixEqu::solveLU(W, k3);

            error = this->error();

            if (error > relTol) {
//                if (dt <= hmin)
                if(t0==t0+dt)
                    throw runtime_error("Step size smaller, than dt_min: "+to_string(dt)+" at t="+to_string(time->getValue()));

                dt = max(hmin, dt * max(0.1, 0.8 * pow(relTol / error, mag)));
                time->setValue(t0);

                noFailed=false;

                nFailed++;
            } else {
                for (int i = 0; i < diffRank; i++) {
                    F0[i] = F2[i];
                    x0[i] = *Xvector[i];
                }
                break;
            }
        }

        if(noFailed){
            double temp = 1.25*pow(error/relTol, mag);
            if (temp > 0.2)
                dt = dt / temp;
            else
                dt = 5.0*dt;
        }
        dt=min(dt,hmax);

        nSteps++;
    }
}
