#include "Demux.h"

using namespace ElementBase;

namespace Elements{
    
    Demux::Demux(){
        id = "Demux";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Environment/Demux.png");
        description = "This block unpacks a vector (to scalars)";

        numOfOutputs = addScalarParameter("numOuts", "Number of outputs", 2);
        numOfOutputs->setOnChange([this](){
            updatePorts();
        });

        inPin = addPin("inp", QPointF(29, 20), Domains::MATH.get(), false, Domains::ConnDirection::Output);

        updatePorts();
    }

    void Demux::updatePorts(){
        // TODO make it proportional to ElemView size!
        size_t oldNum = outputs.size(),
                newNum = numOfOutputs->getScalarValue();

        if(oldNum == newNum)
            return;

        if (newNum > oldNum){
            // simply add new ports
            int gap = 20;
            for (int k=newNum-oldNum; k>0; k--){
                outputs.push_back(addPin("out"+to_string(oldNum+k), Domains::ConnDirection::Input));
            }
        }else{
            // unplug and remove
            // TODO !
        }
    }

    void Demux::discreteInit(){
        if(inPin->dataSize != outputs.size()){
            throw runtime_error("Demux::"+name+" invalid settings. Should have " \
                                +to_string(outputs.size())+ \
                                " outputs, got:"+to_string(inPin->dataSize));
        }
    }

    void Demux::discreteStep(double time){
        (void)time;

        for(auto i=inPin->dataSize; i-->0;){
            *outputs[i]->data = inPin->data[i];
        }
    }

}
