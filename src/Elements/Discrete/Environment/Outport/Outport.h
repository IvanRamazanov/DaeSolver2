#ifndef ELEMS_OUTPORT_H
#define ELEMS_OUTPORT_H

#include "../../../../ElementBase/Element.h"
#include "../../../../ElementBase/Pass.h"

namespace Elements{
    class Outport : public ElementBase::Element, virtual public Pass {
        ElementBase::Pin *in, *out;

        public:
            Outport();

            ElementBase::Pin* getOuter() override;
            ElementBase::Pin* getInner() override;

            void discreteStep(double time) override;
            
    };
}

#endif
