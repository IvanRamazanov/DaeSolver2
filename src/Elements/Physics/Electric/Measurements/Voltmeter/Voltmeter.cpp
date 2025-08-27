#include "Voltmeter.h"

using namespace ElementBase;

namespace Elements{

    Voltmeter::Voltmeter(){
        id = "Voltmeter";
        name = id;
        imgPath = ":/src/data/Elements/Physics/Electric/Measurements/Voltmeter.png";
        description = "This block represents a voltmeter.\n"
                        "Output: voltage in volts";

        // pins
        addPin("p1", QPointF(7, 0), Domains::ELECTRIC.get());
        addPin("p2", QPointF(7, 54), Domains::ELECTRIC.get());
        addPin("out", QPointF(27, 30), Domains::MATH.get(), false, Domains::Output);

        outVar = workspace.get("out");
        p1Var = workspace.get("p1.p");
        p2Var = workspace.get("p2.p");

        // equations
        equations.push_back(string("p1.f"));

        // flow
        flows.push_back({pins[0].get(), pins[1].get()});
    }

    void Voltmeter::discreteStep(double time) {
        outVar->setValue(p1Var->getValue() - p2Var->getValue());
    }

}
