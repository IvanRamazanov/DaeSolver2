/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_SWITCHP_H
#define ELEMENTS_SWITCHP_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class SwitchPath : public ElementBase::Element {
        ElementBase::Pin *outP, *in1P, *in2P, *swtP;
        double swtVal;

        public:
            SwitchPath();
            void discreteStep(double time) override;
            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
