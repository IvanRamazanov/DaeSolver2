#ifndef ELEMS_TRANSF_H
#define ELEMS_TRANSF_H

#include "../../../../ElementBase/Element.h"

namespace Elements{

    class TransferFunction : public ElementBase::Element{
        WorkSpace::Variable *X, *out;
        WorkSpace::DifferentialVar *dX;
        ElementBase::Pin *inP;
        vector<double> p_nom, p_denom;
        size_t lenA, lenB;

        public:
            TransferFunction();

            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
            void discreteStep(double time) override;
            void continuousStep() override;
    };
}

#endif
