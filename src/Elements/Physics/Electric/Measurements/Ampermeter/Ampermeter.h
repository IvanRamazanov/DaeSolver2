#ifndef ELEMS_AMPERMETER_H
#define ELEMS_AMPERMETER_H

#include "../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../ElementBase/Element.h"

namespace Elements{
    class Ampermeter : public ElementBase::Element {
        WorkSpace::Variable *outVar, // output
                            *obsVar; // observable var

        public:
            Ampermeter();

            void discreteStep(double time) override;
    };
}

#endif
