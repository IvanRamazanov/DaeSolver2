#ifndef ELEMS_INTEGRATOR_H
#define ELEMS_INTEGRATOR_H

#include "../../../../ElementBase/Element.h"

namespace Elements{

    class Integrator : public ElementBase::Element{
        WorkSpace::Variable *X, *out;
        WorkSpace::DifferentialVar *dX;
        ElementBase::Pin *inP;

        public:
            Integrator();

            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;

            void continuousStep() override;
    };
}

#endif
