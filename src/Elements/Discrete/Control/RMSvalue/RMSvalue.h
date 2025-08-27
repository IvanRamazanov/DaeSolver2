#ifndef ELEMENTS_RMS_VAL_H
#define ELEMENTS_RMS_VAL_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class RMSvalue : public ElementBase::Element {
        WorkSpace::Variable *X;
        WorkSpace::DifferentialVar *dX;
        ElementBase::Pin *outP, *inP;

        public:
            RMSvalue();
            void discreteStep(double time) override;
            void continuousStep() override;

            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
            
    };
}

#endif
