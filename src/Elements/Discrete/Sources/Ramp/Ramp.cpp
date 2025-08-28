#include "Ramp.h"

using namespace ElementBase;


namespace Elements{
    Ramp::Ramp(){
        id = "Ramp";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Sources/Ramp.png");
        description = "This block represents a linear output, that triggers at given time moment.";

        addPin("val", Domains::Output);

        addScalarParameter("slope", "Slope", 1);
        addScalarParameter("t_on", "ON time", 0);
    }

    void Ramp::discreteInit() {
        slope = parameters[0]->getScalarValue();
        t_on = parameters[1]->getScalarValue();

        v_out = workspace.get("val")->getRef();
    }

    void Ramp::discreteStep(double time){
        if (time >= t_on){
            *v_out = (time-t_on)*slope;
        }else{
            *v_out = 0;
        }
    }
}
