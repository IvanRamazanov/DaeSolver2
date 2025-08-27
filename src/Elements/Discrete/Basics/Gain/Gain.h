/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_GAIN_H
#define ELEMENTS_GAIN_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class Gain : public ElementBase::Element {
        ElementBase::Parameter *p_gain;
        ElementBase::Pin *outP, *inP;
        double* gainCache;

        public:
            Gain();
            void discreteStep(double time) override;
            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
