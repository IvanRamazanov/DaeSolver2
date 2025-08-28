#include "RMSvalue.h"

using namespace ElementBase;

namespace Elements{
    RMSvalue::RMSvalue(){
        id = "RMSvalue";
        name = "RMS";
        setImgPath(":/src/data/Elements/Discrete/Control/RMSvalue.png");
        description = "RMS value";

        inP = addPin("inp", Domains::Input);
        outP = addPin("outp", Domains::Output);

        X = workspace.addVector("X", {0});
        dX = workspace.addDiff("X", WorkSpace::Discrete);
        addInitValue("initVal", "Initial state", {0}, dX);
    }

    void RMSvalue::discreteStep(double time){
        double *v = X->getRef();
        if(time != 0)
            for(auto i=outP->dataSize; i-->0;) {
                outP->data[i] = sqrt(v[i] / time);
            }
        else
            for(auto i=outP->dataSize; i-->0;) {
                outP->data[i] = sqrt(v[i]);
            }
    }

    void RMSvalue::continuousStep() {
        double *v = dX->getRef();
        for (auto i=dX->getSize(); i-->0;)
            v[i] = inP->data[i]*inP->data[i];
    }

    void RMSvalue::discreteInit(){
        if(dX->getInitVal()->getSize() != outP->dataSize)
            throw runtime_error("RMS::"+name+" incorrect initial state size. Expected:"+\
                            to_string(outP->dataSize)+"\n"
                            "got:"+to_string(dX->getInitVal()->getSize()));
    }

    void RMSvalue::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        Element::discreteOutputsInit(inP);
    }
}
