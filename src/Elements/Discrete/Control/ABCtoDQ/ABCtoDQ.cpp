#include "ABCtoDQ.h"
#include <numbers>

using namespace ElementBase;

namespace Elements{
        ABCtoDQ::ABCtoDQ(){
            id = "ABCtoDQ";
            name = "abc-dq0";
            setImgPath(":/src/data/Elements/Discrete/Control/ABCtoDQ.png");
            description = "Park transformation from ABC into dq0\n"
                "First input - ABC vector, Second - axes rotation angle.";

            inP = addPin("inp", Domains::Input);
            thetaP = addPin("inp", Domains::Input);
            thetaP->dataSizeSetting = 1;
            outP = addPin("outp", Domains::Output);
        }

        void ABCtoDQ::discreteStep(double time){
            (void)time;

            double  a = inP->data[0],
                    b = inP->data[1],
                    c = inP->data[2],
                    th = thetaP->data[0];

            // aplha
            outP->data[0] = 2.0/3.0*(cos(th)*a+cos(th-2.0/3.0*numbers::pi)*b+cos(th+2.0/3.0*numbers::pi)*c);
            // beta
            outP->data[1] = 2.0/3.0*(-sin(th)*a-sin(th-2.0/3.0*numbers::pi)*b-sin(th+2.0/3.0*numbers::pi)*c);
            // zero
            outP->data[2] = 1.0/3.0*(a+b+c);
        }

        void ABCtoDQ::discreteOutputsInit(ElementBase::Pin *inheritFrom){
            initPortSize(outP, 3, inP);
            // theta inport - ignore(?)
        }
}
