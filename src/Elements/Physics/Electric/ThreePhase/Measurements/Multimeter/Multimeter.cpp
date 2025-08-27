#include "Multimeter.h"

using namespace ElementBase;

namespace Elements{
    Multimeter::Multimeter(){
        id = "Multimeter";
        name = id;
        imgPath = ":/src/data/Elements/Physics/Electric/ThreePhase/Measurements/Multimeter.png";
        description = "Measures phase currents and line voltage";
        
        // pins
        addPin("p1", QPointF(25, 5), Domains::ELECTRIC_3PH.get());
        addPin("p2", QPointF(25, 66), Domains::ELECTRIC_3PH.get());
        auto p = addPin("outI", QPointF(27, 15), Domains::MATH.get(), false, Domains::Output);
        p->dataSizeSetting = 3; // math pins hold 3 phase data
        p = addPin("outV", QPointF(27, 40), Domains::MATH.get(), false, Domains::Output);
        p->dataSizeSetting = 3; // math pins hold 3 phase data

        outVoltage = workspace.get("outV");
        outCurr = workspace.get("outI");
        A_Var = workspace.get("p1.p[1]");
        B_Var = workspace.get("p1.p[2]");
        C_Var = workspace.get("p1.p[3]");
        Ac_Var = workspace.get("p1.f[1]");
        Bc_Var = workspace.get("p1.f[2]");
        Cc_Var = workspace.get("p1.f[3]");

        // equations
        equations.push_back(string("p1.p[1] - p2.p[1]"));
        equations.push_back(string("p1.p[2] - p2.p[2]"));
        equations.push_back(string("p1.p[3] - p2.p[3]"));

        // flow
        flows.emplace_back(GraphFlowData({"p1.f[1]", "p2.f[1]"}, {pins[0].get(), pins[1].get()}));
        flows.emplace_back(GraphFlowData({"p1.f[2]", "p2.f[2]"}, {pins[0].get(), pins[1].get()}));
        flows.emplace_back(GraphFlowData({"p1.f[3]", "p2.f[3]"}, {pins[0].get(), pins[1].get()}));
    }

    void Multimeter::discreteStep(double time) {
        (void)time;
        
        outCurr->setValue({Ac_Var->getValue(), Bc_Var->getValue(), Cc_Var->getValue()});
        outVoltage->setValue({
            A_Var->getValue() - B_Var->getValue(),
            A_Var->getValue() - C_Var->getValue(),
            B_Var->getValue() - C_Var->getValue()
        });
    }
}
