#ifndef ELEMS_3PH_MULTIMETER_H
#define ELEMS_3PH_MULTIMETER_H

#include "../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../ElementBase/Element.h"

namespace Elements{
    class ThreePhMultimeter : public ElementBase::Element {
        WorkSpace::Variable *outVoltage,
                            *outCurr, // output
                            *A_Var, // observable vars
                            *B_Var,
                            *C_Var,
                            *Ac_Var,
                            *Bc_Var,
                            *Cc_Var; 

        public:
            ThreePhMultimeter();
            
            void discreteStep(double time) override;
    };
}

#endif
