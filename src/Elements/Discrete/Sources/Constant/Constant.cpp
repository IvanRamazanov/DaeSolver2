#include "Constant.h"

using namespace ElementBase;

namespace Elements{
    Constant::Constant(){
        id = "Constant";
        name = id;
        imgPath = ":/src/data/Elements/Discrete/Sources/Constant.png";
        description = "This block represents a constant value.";

        addPin("outp", Domains::Output);

        addVectorParameter("cVal", "Value", {1});
    }

    void Constant::discreteInit() {
        // simply write once into outport
        workspace.get("outp")->setValue(parameters[0]->getVectorValue());
    }

    void Constant::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        size_t size = parameters[0]->getSize();
        initPortSize(pins[0].get(), size);
    }
}
