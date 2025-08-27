#ifndef ELEMENTS_SQUEEZE_WAVE_H
#define ELEMENTS_SQUEEZE_WAVE_H

#include "../../../../ElementBase/Element.h"


/**
 *
 * @author Ivan
 */
namespace Elements{
    class SqeezeWave : public ElementBase::Element {
        double fStart, fEnd, slopeTime, amp, offs, *v_out;

        public:
            SqeezeWave();
            void discreteStep(double time) override;
            void discreteInit() override;
    };
}

#endif
