#ifndef ELEMS_MILTIMETER_3PH_H
#define ELEMS_MILTIMETER_3PH_H

#include "../../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../../ElementBase/Element.h"

namespace Elements{
    class Multimeter : public ElementBase::Element {
        WorkSpace::Variable *outVoltage,
                            *outCurr, // output
                            *A_Var, // observable vars
                            *B_Var,
                            *C_Var,
                            *Ac_Var,
                            *Bc_Var,
                            *Cc_Var; 

        public:
            Multimeter();

            void discreteStep(double time) override;
    };
}

#endif


