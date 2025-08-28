#include "SpeedSensor.h"

using namespace ElementBase;

namespace Elements{

    SpeedSensor::SpeedSensor(){
        id = "SpeedSensor";
        name = "Speed sensor";
        setImgPath(":/src/data/Elements/Physics/Rotational/Measurements/SpeedSensor.png");
        description = "Ideal speed sensor";

        // pins
        addPin("p1", QPointF(17, 0), Domains::MECH_ROTATIONAL.get());
        addPin("p2", QPointF(17, 54), Domains::MECH_ROTATIONAL.get());
        addPin("out", QPointF(27, 30), Domains::MATH.get(), false, Domains::Output);

        outVar = workspace.get("out");
        p1Var = workspace.get("p1.p");
        p2Var = workspace.get("p2.p");

        // equations
        equations.push_back(string("p1.f"));

        // flow
        flows.push_back({pins[0].get(), pins[1].get()});
    }

    void SpeedSensor::discreteStep(double time) {
        (void)time;
        outVar->setValue(p1Var->getValue() - p2Var->getValue());
    }

}
