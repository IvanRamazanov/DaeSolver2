#ifndef ELEMS_SPEED_SENS_H
#define ELEMS_SPEED_SENS_H

#include "../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../ElementBase/Element.h"

namespace Elements{
    class SpeedSensor : public ElementBase::Element {
        WorkSpace::Variable *outVar, // output
                            *p1Var, // observable var
                            *p2Var; 

        public:
            SpeedSensor();
            
            void discreteStep(double time) override;
    };
}

#endif
