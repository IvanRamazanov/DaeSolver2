#include "BDF1.h"

namespace Solvers{

    void BDF1::newtonF(){
        for(int i=0; i<diffRank; i++){
            F[i] = temp[i] - *Xvector[i] + dt * *dXvector[i];
        }
    }

    void BDF1::selfInit() {
        temp = vector<double>(diffRank);
        copyArray(Xvector, temp);

        F = vector<double>(diffRank, 0);
        dF = vector<vector<double>>(diffRank, vector<double>(diffRank, 0));
    }

    void BDF1::estimJ(vector<vector<double>> & J,
                vector<shared_ptr<MathPack::StringGraph>> const& fx,
                vector<double> const& f0,
                vector<double*> & x,
                vector<double> & fac) 
    {
        size_t ny=f0.size();
        for(size_t i=0; i<ny; i++){
            double x0 = *x[i],
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

    void BDF1::evalNextStep() {
        time->setValue(time->getValue()+dt);

        copyArray(Xvector,temp); //save X_(n-1)
        newtonF(); // eval F(X)

        bool isErrnous=false;
        while(MathPack::MatrixEqu::norm(F) > tolerance){
            isErrnous = true;
            vector<double> f0 = F;
            vector<double> dummyVec; // just an empty vector

            estimJ(dF, {}, f0, Xvector, dummyVec);

            MathPack::MatrixEqu::solveLU(dF, f0); // f0 = deltaX

            for(int i=0;i<diffRank;i++){
                *Xvector[i] = *Xvector[i]-f0[i];
            }

            evalSysState();
            newtonF(); // eval F(X)
        }
        if(!isErrnous)
            evalSysState();
    }

}
