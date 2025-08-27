#include "../WorkSpace.h"

namespace WorkSpace{
    DifferentialVar::DifferentialVar(shared_ptr<Variable> baseVar, WsDataType type): 
        Variable(baseVar->getName(), 0, false, type),
        baseVar(baseVar)
    {
        name = DIFF_PTRFIX + name;
    }

    WsTypes DifferentialVar::type() {
        return Differential;
    }

    void DifferentialVar::setInitVal(ElementBase::InitParam *initVal){
        this->initVal = initVal;
    }

    ElementBase::InitParam* DifferentialVar::getInitVal(){
        return initVal;
    }
}
