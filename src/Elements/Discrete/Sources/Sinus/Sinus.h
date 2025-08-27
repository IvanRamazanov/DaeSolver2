/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_SINE_H
#define ELEMENTS_SINE_H

#include "../../../../ElementBase/Element.h"


/**
 *
 * @author Ivan
 */
namespace Elements{
    class Sinus : public ElementBase::Element {
        ElementBase::Parameter *p_amp, *p_freq, *p_offs;
        double amp, freq, offs;

        public:
            Sinus();
            void discreteStep(double time) override;
            void discreteInit() override;
    };
}

#endif
