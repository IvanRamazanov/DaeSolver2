#include "Delay.h"

namespace Elements{
    Delay::Delay(){
        id = name = "Delay";
        description = "Zero-hold";
        setImgPath(":src/data/Elements/Discrete/Continuous/Delay.png");

        // pins
        addPin("in", Domains::Input);
        pins[0]->is_cached = true;
        addPin("out", Domains::Output);
    }
}
