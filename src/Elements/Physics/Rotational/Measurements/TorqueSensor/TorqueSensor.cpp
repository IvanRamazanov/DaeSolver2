#include "TorqueSensor.h"

using namespace ElementBase;

namespace Elements{
    TorqueSensor::TorqueSensor(){
        id = "TorqueSensor";
        name = "Torque sensor";
        imgPath = ":/src/data/Elements/Physics/Rotational/Measurements/TorqueSensor.png";
        description = "Ideal torque sensor";
        
        // pins
        addPin("p1", QPointF(17, 0), Domains::ELECTRIC.get());
        addPin("p2", QPointF(17, 54), Domains::ELECTRIC.get());
        addPin("out", QPointF(27, 30), Domains::MATH.get(), false, Domains::Output);
        outVar = workspace.get("out");
        obsVar = workspace.get("p1.f");

        // equations
        equations.push_back(string("p1.p - p2.p"));

        // flow
        flows.push_back({pins[0].get(), pins[1].get()});
    }

    void TorqueSensor::discreteStep(double time) {
        outVar->setValue(obsVar->getValue());
    }
}
