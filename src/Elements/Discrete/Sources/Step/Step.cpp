#include "Step.h"

using namespace ElementBase;


namespace Elements{
    Step::Step(){
        id = "Step";
        name = id;
        imgPath = ":/src/data/Elements/Discrete/Sources/Step.png";
        description = "This block represents a step function output.\n"
            "It changes value from 'level off' to 'level on' at 'Switch time' moment of time.";

        addPin("val", Domains::Output);

        addScalarParameter("v_on", "level on", 1);
        addScalarParameter("v_off", "level off", 0);
        addScalarParameter("t_on", "Switch time", 0.5);
    }

    void Step::discreteInit() {
        v_on = parameters[0]->getScalarValue();
        v_off = parameters[1]->getScalarValue();
        t_on = parameters[2]->getScalarValue();

        v_out = workspace.get("val")->getRef();
    }

    void Step::discreteStep(double time){
        if (time >= t_on){
            *v_out = v_on;
        }else{
            *v_out = v_off;
        }
    }
}
