#include "Ampermeter.h"

using namespace ElementBase;

namespace Elements{
    Ampermeter::Ampermeter(){
        id = "Ampermeter";
        name = id;
        imgPath = ":/src/data/Elements/Physics/Electric/Measurements/Ampermeter.png";
        description = "This block represents an ampermeter.\n"
                        "Output: current in amperes";
        
        // pins
        addPin("p1", QPointF(7, 0), Domains::ELECTRIC.get());
        addPin("p2", QPointF(7, 54), Domains::ELECTRIC.get());
        addPin("out", QPointF(27, 30), Domains::MATH.get(), false, Domains::Output);
        outVar = workspace.get("out");
        obsVar = workspace.get("p1.f");

        // equations
        equations.push_back(string("p1.p - p2.p"));

        // flow
        flows.push_back({pins[0].get(), pins[1].get()});
    }

    void Ampermeter::discreteStep(double time) {
        outVar->setValue(obsVar->getValue());
    }
}
