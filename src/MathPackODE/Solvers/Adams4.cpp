#include "Adams4.h"

namespace Solvers{
    Adams4::Adams4(){
    }

    void Adams4::selfInit() {
        matrStep = 0;
        ds = vector<vector<double>>(diffRank, vector<double>(rank, 0));
    }

    void Adams4::evalNextStep() {
        if(fullfill){
            for(int i=0; i<diffRank; i++){
                ds[i][matrStep] = *dXvector[i];
            }
            for(int i=0; i<diffRank; i++){
                double val = *Xvector[i]+dt*(55.0/24.0*ds[i][matrStep]-59.0/24.0*ds[i][(matrStep+3)%rank]
                        +37.0/24.0*ds[i][(matrStep+2)%rank]-3.0/8.0*ds[i][(matrStep+1)%rank]);
                *Xvector[i] = val;
            }
            matrStep = (matrStep+1)%rank;
        }else{
            for(int i=0;i<diffRank;i++){
                ds[i][matrStep] = *dXvector[i];
            }
            for(int i=0; i<diffRank; i++){
                *Xvector[i] = ds[i][matrStep]*dt + *Xvector[i];
            }

            if(++matrStep==rank-1)
                fullfill=true;
        }

        time->setValue(time->getValue()+dt);
        evalSysState(); //Newton update
    }
}
