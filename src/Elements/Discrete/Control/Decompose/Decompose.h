/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_DECOMPOSE_H
#define ELEMENTS_DECOMPOSE_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class Decompose : public ElementBase::Element {
        ElementBase::Pin *outMag, *outDeg, *inP;

        public:
            Decompose();
            void discreteStep(double time) override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
