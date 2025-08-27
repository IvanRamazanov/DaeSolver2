/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_ABC2AB0_H
#define ELEMENTS_ABC2AB0_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class ABCtoAB0 : public ElementBase::Element {
        ElementBase::Pin *outP, *inP;

        public:
            ABCtoAB0();
            void discreteStep(double time) override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
