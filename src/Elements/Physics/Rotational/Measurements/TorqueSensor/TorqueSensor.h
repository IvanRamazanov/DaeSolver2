#ifndef ELEMS_TORQUE_SENSOR_H
#define ELEMS_TORQUE_SENSOR_H

#include "../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../ElementBase/Element.h"

namespace Elements{
    class TorqueSensor : public ElementBase::Element {
        WorkSpace::Variable *outVar, // output
                            *obsVar; // observable var

        public:
            TorqueSensor();

            void discreteStep(double time) override;
    };
}

#endif
