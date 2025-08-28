#include "SimulationTime.h"

using namespace ElementBase;


namespace Elements{
    SimulationTime::SimulationTime(){
        id = "SimulationTime";
        name = "Simulation time";
        setImgPath(":/src/data/Elements/Discrete/Sources/SimulationTime.png");
        description = "This block represents a time clock.\n"
                "Output value equals simulation time.";

        addPin("val", Domains::Output);

        addScalarParameter("v_on", "level on", 1);
        addScalarParameter("v_off", "level off", 0);
        addScalarParameter("t_on", "Switch time", 0.5);
    }

    void SimulationTime::discreteInit() {
        v_out = workspace.get("val")->getRef();
    }

    void SimulationTime::discreteStep(double time){
        *v_out = time;
    }
}
