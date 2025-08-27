#include "Sinus.h"
#include <numbers>

using namespace ElementBase;

constexpr double pi = std::numbers::pi;

namespace Elements{
        Sinus::Sinus(){
            id = "Sinus";
            name = "Sine wave";
            imgPath = ":/src/data/Elements/Discrete/Sources/Sinus.png";
            description = "This block represents a sinus wave source.";

            addPin("val", Domains::Output);

            p_amp = addScalarParameter("amp", "Amplitude", 1);
            p_freq = addScalarParameter("f", "Frequency, Hz", 1);
            p_offs = addScalarParameter("offs", "Phase shift, deg", 0);

            discreteFunction = "val = amp*sin(2*"+to_string(pi)+"*f*t+offs/180*"+to_string(pi)+");";
        }

        void Sinus::discreteInit() {
            amp = p_amp->getScalarValue();
            freq = p_freq->getScalarValue();
            offs = p_offs->getScalarValue();
        }

        void Sinus::discreteStep(double time){
            *pins[0]->data = amp*sin(2.0*pi*freq*time+offs/180*pi);
        }
}
