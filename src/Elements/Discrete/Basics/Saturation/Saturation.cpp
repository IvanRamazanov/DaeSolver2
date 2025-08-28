#include "Saturation.h"

using namespace ElementBase;

namespace Elements{
    Saturation::Saturation(){
        id = "Saturation";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Basics/Saturation.png");
        description = "This block represents a saturation.";

        inP = addPin("inp", Domains::Input);
        outP = addPin("outp", Domains::Output);

        p_up = addScalarParameter("upLim", "Upper limit", 1);
        p_down = addScalarParameter("loLim", "Lower limit", -1);
    }

    void Saturation::discreteInit() {
        upLim = p_up->getScalarValue();
        downLim = p_down->getScalarValue();
    }

    void Saturation::discreteStep(double time){
        (void)time;

        for (size_t i=outP->dataSize; i-->0;)
            outP->data[i] = min(upLim, max(downLim, inP->data[i]));
    }

    void Saturation::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        Element::discreteOutputsInit(inP);
    }
}
