#include "SqeezeWave.h"
#include <numbers>

using namespace ElementBase;


namespace Elements{
    SqeezeWave::SqeezeWave(){
        id = "SqeezeWave";
        name = "Squeeze sine wave";
        setImgPath(":/src/data/Elements/Discrete/Sources/SqeezeWave.png");
        description = "This block represents sine wave with linearly increased frequency.";

        addPin("val", Domains::Output);

        addScalarParameter("fStart", "Starting frequency, Hz", 0);
        addScalarParameter("fEnd", "Final frequency, Hz", 50);
        addScalarParameter("slopeTime", "Raise time, sec", 1);
        addScalarParameter("amp", "Amplitude", 1);
        addScalarParameter("offs", "Phase shift, deg", 0);
    }

    void SqeezeWave::discreteInit() {
        fStart = parameters[0]->getScalarValue();
        fEnd = parameters[1]->getScalarValue();
        slopeTime = parameters[2]->getScalarValue();
        amp = parameters[3]->getScalarValue();
        offs = parameters[4]->getScalarValue();

        v_out = workspace.get("val")->getRef();
    }

    void SqeezeWave::discreteStep(double time){
        if(time < slopeTime){
            double smoothA = (amp-15)/slopeTime*time + 15;
            *v_out = smoothA*sin(2*numbers::pi*((fEnd-fStart)/slopeTime*time+fStart)/2.0*time+offs/180.0*numbers::pi);
        }else
            *v_out = amp*sin(2.0*numbers::pi*fEnd*time + offs*numbers::pi/180.0);
    }
}
