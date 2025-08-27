#ifndef ELEMS_INPORT_H
#define ELEMS_INPORT_H

#include "../../../../ElementBase/Element.h"
#include "../../../../ElementBase/Pass.h"

namespace Elements{
    class Inport : public ElementBase::Element, virtual public Pass {
        ElementBase::Pin *in, *out;

        public:
            Inport();

            ElementBase::Pin* getOuter() override;
            ElementBase::Pin* getInner() override;

            void discreteStep(double time) override;

    };
}

#endif
