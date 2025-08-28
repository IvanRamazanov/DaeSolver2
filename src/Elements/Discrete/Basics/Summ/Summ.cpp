#include "Summ.h"

using namespace ElementBase;

namespace Elements{
    Sum::Sum(){
        id = "Sum";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Basics/Sum.png");
        description = "This block calculates sum of Inputs.";

        inP1 = addPin("in1", Domains::Input);
        inP2 = addPin("in2", Domains::Input);
        outP = addPin("out", Domains::Output);

        gains = addVectorParameter("gains", "Gains", {1, -1});
    }

    void Sum::discreteStep(double time){
        (void)time;

        for (size_t i=outP->dataSize; i-->0;)
            outP->data[i] = inP1->data[i] + gainsCache[i]*inP2->data[i];
    }

    void Sum::discreteInit(){
        // verify that size of gains == num of inputs
        if(gains->getSize() != 2)
            throw runtime_error("Sum::"+name+" incorrect Gains parameter: expected size=2\n"
                                "got:"+to_string(gains->getSize()));

        gainsCache = workspace.get("gains")->getRef();
    }

    void Sum::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        initPortSize(outP, inP1->dataSize, inP2);
    }
}
