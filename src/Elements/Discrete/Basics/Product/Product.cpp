#include "Product.h"

using namespace ElementBase;

namespace Elements{
    Product::Product(){
        id = "Product";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Basics/Product.png");
        description = "out=in1*in2";

        inP1 = addPin("in1", Domains::Input);
        inP2 = addPin("in2", Domains::Input);
        outP = addPin("out", Domains::Output);
    }

    void Product::discreteStep(double time){
        (void)time;

        for (size_t i=outP->dataSize; i-->0;)
            outP->data[i] = inP1->data[i] * inP2->data[i];
    }

    void Product::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        initPortSize(outP, inP1->dataSize, inP2);
    }
}