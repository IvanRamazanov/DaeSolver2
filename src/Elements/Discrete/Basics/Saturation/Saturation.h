/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_SATURATION_H
#define ELEMENTS_SATURATION_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class Saturation : public ElementBase::Element {
        ElementBase::Parameter *p_up, *p_down;
        ElementBase::Pin *outP, *inP;
        double upLim, downLim;

        public:
            Saturation();
            void discreteStep(double time) override;
            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
