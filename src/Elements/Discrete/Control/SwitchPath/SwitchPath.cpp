#include "SwitchPath.h"

using namespace ElementBase;

namespace Elements{
        SwitchPath::SwitchPath(){
            id = "SwitchPath";
            name = "Switch";
            setImgPath(":/src/data/Elements/Discrete/Control/SwitchPath.png");
            description = "If second input's value bigger, than threshold, then first input passes.";

            in1P = addPin("inp1", Domains::Input);
            swtP = addPin("swt", Domains::Input);
            swtP->dataSizeSetting = 1;
            in2P = addPin("inp2", Domains::Input);
            outP = addPin("outp", Domains::Output);

            addScalarParameter("swtThrsh", "Threshold", 0);
        }

        void SwitchPath::discreteStep(double time){
            (void)time;

            if(swtP->data[0] > swtVal)
                for(auto i=outP->dataSize; i-->0;)
                    outP->data[i] = in1P->data[i];
            else
                for(auto i=outP->dataSize; i-->0;)
                    outP->data[i] = in2P->data[i];
        }

        void SwitchPath::discreteInit() {
            swtVal = parameters[0]->getScalarValue();
        }

        void SwitchPath::discreteOutputsInit(ElementBase::Pin *inheritFrom){
            Element::discreteOutputsInit(in1P);

            if (in1P->dataSize != in2P->dataSize)
                throw runtime_error("Switch::"+name+" input 1 and 3 must have same sizes!");
        }
}
