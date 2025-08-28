#include "PIDController.h"

using namespace ElementBase;

namespace Elements{
    PIDController::PIDController(){
        id = "PIDController";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Control/PIDController.png");
        description = "This block represents PID-controller";

        inP = addPin("inp", Domains::Input);
        outP = addPin("outp", Domains::Output);

        X = workspace.addVector("X", {0});
        dX = workspace.addDiff("X", WorkSpace::Discrete);
        addInitValue("initVal", "Initial state", {0}, dX);

        addScalarParameter("Kp", "Proportional coeff", 1);
        addScalarParameter("Ki", "Integration coeff", 1);
        addScalarParameter("Kd", "Differentiation coeff", 0);
        
    }

    void PIDController::discreteStep(double time){
        for (auto i=outP->dataSize; i-->0;){
            double kp = K_p*inP->data[i],
                    kd = K_d*(inP->data[i]-inp_old[i])/(time-T_old),
                    ki = K_i*X->getRef()[i];
            outP->data[i] = kp+ki+kd;
        }
        
        T_old = time;

        std::memcpy(inp_old.data(), inP->data, inP->dataSize*sizeof(double));
    }

    void PIDController::continuousStep() {
        memcpy(dX->getRef(), inP->data, dX->getSize()*sizeof(double));
    }

    void PIDController::discreteInit(){
        K_i = parameters[1]->getScalarValue();
        K_p = parameters[0]->getScalarValue();
        K_d = parameters[2]->getScalarValue();

        if(dX->getInitVal()->getSize() != outP->dataSize)
            throw runtime_error("PID::"+name+" incorrect initial state size. Expected:"+\
                            to_string(outP->dataSize)+"\n"
                            "got:"+to_string(dX->getInitVal()->getSize()));

        T_old = 1; // to guard against div by 0

        inp_old.resize(inP->dataSize);
        std::memcpy(inp_old.data(), inP->data, inP->dataSize*sizeof(double));
    }

    void PIDController::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        Element::discreteOutputsInit(inP);
    }
}
