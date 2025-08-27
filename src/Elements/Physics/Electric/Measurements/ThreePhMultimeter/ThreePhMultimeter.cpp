#include "ThreePhMultimeter.h"

using namespace ElementBase;

namespace Elements{

    ThreePhMultimeter::ThreePhMultimeter(){
        id = "ThreePhMultimeter";
        name = id;
        imgPath = ":/src/data/Elements/Physics/Electric/Measurements/ThreePhMultimeter.png";
        description = "This block measures three-phase currents and line-line voltages.";

        // pins
        addPin("A", QPointF(4, 4), Domains::ELECTRIC.get());
        addPin("B", QPointF(20, 4), Domains::ELECTRIC.get());
        addPin("C", QPointF(40, 4), Domains::ELECTRIC.get());
        addPin("a", QPointF(4, 60), Domains::ELECTRIC.get());
        addPin("b", QPointF(20, 60), Domains::ELECTRIC.get());
        addPin("c", QPointF(40, 60), Domains::ELECTRIC.get());
        auto p1 = addPin("outV", QPointF(20, 60), Domains::MATH.get(), false, Domains::Output);
        auto p2 = addPin("outC", QPointF(40, 60), Domains::MATH.get(), false, Domains::Output);

        outVoltage = workspace.get("outV");
        outCurr = workspace.get("outC");
        A_Var = workspace.get("A.p");
        B_Var = workspace.get("B.p");
        C_Var = workspace.get("C.p");
        Ac_Var = workspace.get("A.f");
        Bc_Var = workspace.get("B.f");
        Cc_Var = workspace.get("C.f");

        p1->dataSizeSetting = 3; // math pins hold 3 phase data
        p2->dataSizeSetting = 3;

        // equations
        equations.push_back(string("p.1 - p.4"));
        equations.push_back(string("p.2 - p.5"));
        equations.push_back(string("p.3 - p.6"));

        // flow
        flows.push_back({pins[0].get(), pins[3].get()});
        flows.push_back({pins[1].get(), pins[4].get()});
        flows.push_back({pins[2].get(), pins[5].get()});
    }

    void ThreePhMultimeter::discreteStep(double time) {
        (void)time;
        
        outCurr->setValue({Ac_Var->getValue(), Bc_Var->getValue(), Cc_Var->getValue()});
        outVoltage->setValue({
            A_Var->getValue() - B_Var->getValue(),
            A_Var->getValue() - C_Var->getValue(),
            B_Var->getValue() - C_Var->getValue()
        });
    }

}
