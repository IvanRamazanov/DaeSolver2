/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_SUM_H
#define ELEMENTS_SUM_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class Sum : public ElementBase::Element {
        ElementBase::Parameter *gains;
        ElementBase::Pin *outP, *inP1, *inP2;
        double *gainsCache;

        public:
            Sum();
            void discreteStep(double time) override;
            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
