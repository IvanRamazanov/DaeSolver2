#include "RK4.h"

namespace Solvers{
    void RK4::add(int step){
        time->setValue(bTime+C[step]*dt);

        for(size_t i=0; i<diffRank; i++){
            double sum=0;
            for(int j=0;j<step;j++){
                sum+=K[j][i]*A[step-1][j];
            }
            *Xvector[i] = bXVector[i]+dt*sum;
        }
    }

    void RK4::selfInit() {
        bXVector = vector<double>(diffRank, 0);

        K = vector<vector<double>>(ORDER, vector<double>(diffRank, 0));
    }

    RK4::RK4(){}

    void RK4::evalNextStep() {
        bTime = time->getValue();
        copyArray(Xvector,bXVector);  //save X(t0)

        // first step
        copyArray(dXvector, K[0]);  //save K1=dXi(t0+h)
        // n-th step
        for(size_t i=1; i<ORDER; i++) {
            add(i);
            evalSysState();
            copyArray(dXvector, K[i]);
        }

        for(size_t i=0; i<diffRank; i++){
            double sum=0;
            for(size_t j=0; j<K.size(); j++) {
                sum += B[j] * K[j][i];
            }
            *Xvector[i] = bXVector[i]+dt*sum;
        }

        time->setValue(bTime+dt);

        evalSysState(); // for correct outputs
    }
}
