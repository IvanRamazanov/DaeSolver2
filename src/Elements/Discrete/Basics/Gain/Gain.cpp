#include "Gain.h"

using namespace ElementBase;

namespace Elements{
        Gain::Gain(){
            id = "Gain";
            name = id;
            setImgPath(":/src/data/Elements/Discrete/Basics/Gain.png");
            description = "This block represents an amplifier.\n"
                            "Output = Gain * Input.";

            inP = addPin("inp", Domains::Input);
            outP = addPin("outp", Domains::Output);

            p_gain = addVectorParameter("gain", "Gain", {1});
        }

        void Gain::discreteInit() {
            gainCache = workspace.get("gain")->getRef();
        }

        void Gain::discreteStep(double time){
            (void)time;

            for (size_t i=outP->dataSize; i-->0;)
                outP->data[i] = gainCache[i] * inP->data[i];
        }

        void Gain::discreteOutputsInit(ElementBase::Pin *inheritFrom){
            initPortSize(outP, p_gain->getSize(), inP);
        }
}
