#include "Integrator.h"

namespace Elements{
    Integrator::Integrator(){
        id = "Integrator";
        name = id;
        setImgPath(":src/data/Elements/Discrete/Continuous/Integrator.png");
        description = "This block represents an ideal integration unit.";
        
        inP = addPin("in", QPointF(0, 19), Domains::MATH.get(), false, Domains::Input, true);
        addPin("out", QPointF(33, 19), Domains::MATH.get(), false, Domains::Output);

        X = workspace.get("in");
        out = workspace.get("out");
        dX = workspace.addDiff("out", WorkSpace::Discrete);

        addInitValue("initVal", "Initial value", {0}, dX);
    }

    void Integrator::continuousStep() {
        memcpy(dX->getRef(), inP->data, dX->getSize()*sizeof(double));
    }

    void Integrator::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        Element::discreteOutputsInit(inP);
    }
}
