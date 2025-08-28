#include "TransferFunction.h"

namespace Elements{
    TransferFunction::TransferFunction(){
        id = "TransferFunction";
        name = id;
        setImgPath(":src/data/Elements/Discrete/Continuous/TransferFunction.png");
        description = "Transfer function";
        
        inP = addPin("in", QPointF(0, 19), Domains::MATH.get(), false, Domains::Input);
        inP->dataSizeSetting = 1;
        addPin("out", QPointF(33, 19), Domains::MATH.get(), false, Domains::Output);

        out = workspace.get("out");
        X = workspace.get("X");
        dX = workspace.addDiff("X", WorkSpace::Discrete);

        addInitValue("initVal", "Initial value", {0}, dX);

        addVectorParameter("denom", "Denominator", {1, 1});
        addVectorParameter("nom", "Nominator", {1});
    }

    void TransferFunction::discreteInit() {
        lenA = parameters[0]->getSize();
        lenB = parameters[1]->getSize();

        if(lenB>lenA)
            throw runtime_error("TF::"+name+" Nominator is bigger, than denominator");
        
        p_denom = vector(parameters[0]->getVectorValue());
        p_nom = vector(parameters[1]->getVectorValue());
        
        if(lenB<lenA){
            vector<double> tmp(lenA);
            for(int i=0;i<lenB;i++){
                tmp[lenA-1-i] = p_nom[lenB-1-i];
            }
            p_nom = tmp;
        }

        // normalization
        double n = p_denom[0];
        for(int i=0;i<lenA;i++)
            p_denom[i] /= n;
        for(int i=0;i<lenB;i++)
            p_nom[i] /= n;

        if((lenA+1) != dX->getInitVal()->getSize())
            throw runtime_error("TF::"+name+" Initial X vector is bigger than TF order!");
    }

    void TransferFunction::discreteStep(double time){
        (void)time;

        double outV=0;
        for(int i=0; i<lenB-1; i++){
            outV += X->getRef()[i]*(p_nom[lenB-i-1]-p_denom[lenB-i-1]*p_nom[0]);
        }
        out->setValue(outV + p_nom[0]*inP->data[0]);
    }

    void TransferFunction::continuousStep() {
        double *wsDX = dX->getRef(),
                *wsX = X->getRef();
        size_t order = dX->getSize();

        // ??
        double sum=0;
        for(int i=1; i<order; i++){
            wsDX[i-1] = wsX[i];
            sum -= p_denom[lenA-i-1]*wsX[i];
        }
        sum -= p_denom[lenA-1]*wsX[0];
        sum += inP->data[0];

        wsDX[order-1] = sum;
    }

    void TransferFunction::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        Element::discreteOutputsInit(inP);
    }
}
