/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ELEMENTS_PID_CONTROLLER_H
#define ELEMENTS_PID_CONTROLLER_H

#include "../../../../ElementBase/Element.h"


namespace Elements{
    class PIDController : public ElementBase::Element {
        WorkSpace::Variable *X;
        WorkSpace::DifferentialVar *dX;
        ElementBase::Pin *outP, *inP;
        double K_i, K_p, K_d, T_old;
        vector<double> inp_old;

        public:
            PIDController();
            void discreteStep(double time) override;
            void continuousStep() override;

            void discreteInit() override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
            
    };
}

#endif
