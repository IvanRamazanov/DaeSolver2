#include "TR.h"

namespace Solvers{
    void TR::makeTRJ(vector<vector<double>> & J){
        for(int i=0;i<diffRank;i++){
            for(int j=0;j<diffRank;j++){
                double add=i==j?1.0:0.0;
                J[i][j]=add-dt/2.0*J[i][j];
            }
        }
    }

    /**
     * fng=dXvector
     * xng=Xvector
     */
    void TR::trF(){
        for (int i = 0; i < diffRank; i++) {
            df1[i] = *dXvector[i];
            newtonF[i] = (*Xvector[i]-x0[i])-dt/2.0*(df1[i]+df0[i]);
        }
    }

    /**
     * Estimates d(d.X)/dX
     * @param J - output
     * @param fx - target function
     * @param f0 - function value at fx(x0)
     * @param x - x0 initial point
     * @param fac
     */
    void TR::estimJ(vector<vector<double>> & J,
                vector<shared_ptr<MathPack::StringGraph>> const& fx,
                vector<double> const& f0,
                vector<double*> & x,
                vector<double> & fac) {
        int ny=f0.size();
        for(int i=0;i<ny;i++){
            double x0 = *x[i],
                    del = x0*1e-3;
            del = del==0?1e-6:del;
            *x[i] = x0+del;

            //evalSysState();

            for(int j=0;j<ny;j++){
                J[j][i] = (*dXvector[j]-f0[j])/del;
            }

            *x[i] = x0;
        }
    } 

    void TR::selfInit() {
        x0 = vector<double>(diffRank, 0);
        copyArray(Xvector, x0);

        df0 = vector<double>(diffRank, 0);
        copyArray(dXvector, df0);
        df1 = vector<double>(diffRank, 0);

        newtonF = vector<double>(diffRank, 0);

        dfdx = vector<vector<double>>(diffRank, vector<double>(diffRank, 0));
    }

    void TR::evalNextStep() {
        int cnt=0;
        double t0=time->getValue();
        time->setValue(t0+dt);
        vector<double> dummy;
        //  TR step
        estimJ(dfdx, {}, df0, Xvector, dummy); // estimate d(d.X)/dX
        makeTRJ(dfdx);

        while(true){

            trF();
            double err = MathPack::MatrixEqu::norm(newtonF);
            while (err > tolerance) {
                cnt++;
                MathPack::MatrixEqu::solveLU(dfdx, newtonF); //  dF*deltaX=f0
                for (int i = 0; i < diffRank; i++) {
                    double val = *Xvector[i] - newtonF[i];
                    *Xvector[i] = val;
                }

                evalSysState();
                trF(); // eval F(X)

                err = MathPack::MatrixEqu::norm(newtonF);
                if(cnt>100)
                    throw runtime_error("Bad TR Newton!");
                if(cnt>20){
                    estimJ(dfdx, {}, df1, Xvector, dummy); // estimate d(d.X)/dX
                    makeTRJ(dfdx);
                    continue;
                }

                if (err > tolerance) {
                } else {
                    break;
                }
            }

            break;
        }
        copyArray(Xvector,x0);
        memcpy(df0.data(), df1.data(), diffRank*sizeof(double));
    }
}
