#include "BDF2.h"

namespace Solvers{
    void BDF2::newtonF(){
        if(xOld.empty()) {
            for (int i = 0; i < diffRank; i++) {
                F[i] = xTemp[i] - *Xvector[i] + dt * *dXvector[i];
            }
        }else{
            for (int i = 0; i < diffRank; i++) {
                F[i] = 4.0/3.0*xTemp[i] - *Xvector[i] -1.0/3.0*xOld[i] + 2.0/3.0* dt * *dXvector[i];
            }
        }
    }

    void BDF2::selfInit() {
        xTemp = vector<double>(diffRank,0);
        copyArray(Xvector,xTemp);

        F = vector<double>(diffRank,0);
        dF = vector<vector<double>>(diffRank, vector<double>(diffRank, 0));

        xOld.clear();
    }

    void BDF2::estimJ(vector<vector<double>> & J,
                vector<shared_ptr<MathPack::StringGraph>> const& fx,
                vector<double> const& f0,
                vector<double*> & x,
                vector<double> & fac) 
    {
        size_t ny = f0.size();
        for(size_t i=0; i<ny; i++){
            double x0=*x[i],
                    del=x0*1e-3;
            del = del==0 ? 1e-6 : del;
            *x[i] = x0+del;

            evalSysState();
            newtonF();

            for(size_t j=0; j<ny; j++){
                J[j][i]=(F[j]-f0[j])/del;
            }

            *x[i] = x0;
        }
    }

    void BDF2::evalNextStep() {
        time->setValue(time->getValue()+dt);

        newtonF(); // eval F(X)
        vector<double> del = F,
            dummy;
        estimJ(dF, {}, del, Xvector, dummy);

//        double err=MatrixEqu.norm(F);
        double err;
        do {
            vector<double> f0 = F;

            MathPack::MatrixEqu::solveLU(dF, del); //  dF*deltaX=f0

            for (int i = 0; i < diffRank; i++) {
                *Xvector[i] = *Xvector[i] - del[i];
            }

            evalSysState();
            newtonF(); // eval F(X)

            err = MathPack::MatrixEqu::norm(F);
            if (err > tolerance){
                for (int i = 0; i < diffRank; i++)
                    del[i] *= -1.0;

                updateJ(dF, F, f0, del);

                del = F;
            }else{
                break;
            }
        }while(err > tolerance);

        xOld = xTemp; //save X_(n-2)
        copyArray(Xvector, xTemp); //save X_(n-1)

    }
}
