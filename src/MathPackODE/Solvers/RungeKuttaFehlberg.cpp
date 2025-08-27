#include "RungeKuttaFehlberg.h"

namespace Solvers{
    void RungeKuttaFehlberg::add(int step){
        time->setValue(bTime+C[step]*dt);

        for(int i=0; i<diffRank; i++){
            double sum=0;
            for(int j=0; j<step; j++){
                sum += K[j][i]*A[step-1][j];
            }
            *Xvector[i] = bXVector[i]+dt*sum;
        }
    }

    void RungeKuttaFehlberg::selfInit() {
        bXVector = vector<double>(diffRank, 0);
        bCommVect = vector<double>(stateVector.size(), 0);

        K = vector<vector<double>>(ORDER, vector<double>(diffRank, 0));
        error = vector<double>(diffRank, 0);
        relErr = vector<double>(diffRank, 0);
    }


    RungeKuttaFehlberg::RungeKuttaFehlberg(){
    }

    void RungeKuttaFehlberg::evalNextStep() {
        bTime = time->getValue();
        copyArray(Xvector,bXVector);  //save X(t0)
        copyArray(stateVector,bCommVect);

        // first step
        copyArray(dXvector,K[0]);  //save K1=dXi(t0+h)
        // n-th step
        for(int i=1; i<ORDER; i++) {
            add(i);
            evalSysState();
            copyArray(dXvector, K[i]);
        }

        double denum=1e-3;
        for(int i=0; i<diffRank; i++){
            double sum=0,errSum=0;
            for(size_t j=0; j<K.size(); j++) {
                sum += B[j] * K[j][i];
                errSum += abs((B[j]-Br[j])*K[j][i]);
            }
            double val = bXVector[i]+dt*sum;
            denum = max(max(abs(val), abs(*Xvector[i])), denum);
            *Xvector[i] = val;


            error[i] = errSum*dt;

            relErr[i] = error[i];
        }

        double err=-1;
        for(int i=0; i<relErr.size(); i++){
            err = max(relErr[i]/denum, err);
        }
//        double err= MatrixEqu.norm(error);

//        double alpha = Math.pow(absTol/err,1.0/(ORDER+1.0));
        double alpha = 1.0 / ( 1.25 * pow(err/relTol, 1.0/(5.0)) );
        if(err>relTol){
            //Bad try. Recalculate
            time->setValue(bTime);
            for(int i=0; i<diffRank; i++){
                *Xvector[i] = bXVector[i];
            }
            for(size_t i=0; i<bCommVect.size(); i++)
                *stateVector[i] = bCommVect[i];

            alpha *= 0.8;
            dt = alpha*dt;
        }else{
            time->setValue(bTime + dt);
            dt = alpha*dt;

            evalSysState(); // for correct outputs
        }

    }
}
