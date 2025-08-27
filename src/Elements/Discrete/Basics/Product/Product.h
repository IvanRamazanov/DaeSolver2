/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_PRODUCT_H
#define ELEMENTS_PRODUCT_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class Product : public ElementBase::Element {
        ElementBase::Pin *outP, *inP1, *inP2;

        public:
            Product();
            void discreteStep(double time) override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
