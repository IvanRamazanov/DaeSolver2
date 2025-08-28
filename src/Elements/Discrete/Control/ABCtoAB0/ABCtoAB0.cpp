#include "ABCtoAB0.h"

using namespace ElementBase;

namespace Elements{
        ABCtoAB0::ABCtoAB0(){
            id = "ABCtoAB0";
            name = "abc-ab0";
            setImgPath(":/src/data/Elements/Discrete/Control/ABCtoAB0.png");
            description = "Park transformation from ABC into AlphaBeta0";

            inP = addPin("inp", Domains::Input);
            outP = addPin("outp", Domains::Output);
        }

        void ABCtoAB0::discreteStep(double time){
            (void)time;

            double  a = inP->data[0],
                    b = inP->data[1],
                    c = inP->data[2];

            // aplha
            outP->data[0] = 2.0/3.0*a-1.0/3.0*b-1.0/3.0*c;
            // beta
            outP->data[1] = 0.5773502691896257*b-0.5773502691896257*c;
            // zero
            outP->data[2] = 1.0/3.0*(a+b+c);
        }

        void ABCtoAB0::discreteOutputsInit(ElementBase::Pin *inheritFrom){
            initPortSize(outP, 3, inP);
        }
}
