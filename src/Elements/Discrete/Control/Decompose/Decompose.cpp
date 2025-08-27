#include "Decompose.h"
#include <numbers>

using namespace ElementBase;

namespace Elements{
        Decompose::Decompose(){
            id = "Decompose";
            name = id;
            imgPath = ":/src/data/Elements/Discrete/Control/Decompose.png";
            description = "Complex to magnitude and degree";

            inP = addPin("inp", Domains::Input);
            inP->dataSizeSetting = 2;
            outMag = addPin("outp", Domains::Output);
            outDeg = addPin("outp", Domains::Output);
        }

        void Decompose::discreteStep(double time){
            (void)time;

            outMag->data[0] = sqrt(inP->data[0]*inP->data[0]+inP->data[1]*inP->data[1]);

            if(inP->data[0] == 0.0){
                if(inP->data[1] < 0)
                    outDeg->data[0] = numbers::pi/2.0;
                else
                    outDeg->data[0] = 0;
            }else{
                outDeg->data[0] = atan(inP->data[1] / inP->data[0]);
            }
        }

        void Decompose::discreteOutputsInit(ElementBase::Pin *inheritFrom){
            Element::discreteOutputsInit();
        }
}
