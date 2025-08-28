#include "Mux.h"

using namespace ElementBase;

namespace Elements{
    
    Mux::Mux(){
        id = "Mux";
        name = id;
        setImgPath(":/src/data/Elements/Discrete/Environment/Mux.png");
        description = "This block represents a vector(or scalar) packing.";

        numOfInputs = addScalarParameter("numInputs", "Number of inputs", 2);
        numOfInputs->setOnChange([this](){
            updatePorts();
        });

        outPin = addPin("out", QPointF(29, 20), Domains::MATH.get(), false, Domains::ConnDirection::Output);
        outVar = workspace.get("out");

        updatePorts();
    }

    void Mux::updatePorts(){
        // TODO make it proportional to ElemView size!
        size_t oldNum=inputs.size(),
                newNum=numOfInputs->getScalarValue();

        if(oldNum == newNum)
            return;

        if (newNum > oldNum){
            // simply add new ports
            int gap = 20;
            for (int k=newNum-oldNum; k>0; k--){
                inputs.push_back(addPin("in"+to_string(oldNum+k), Domains::ConnDirection::Input));
            }
        }else{
            // unplug and remove
            // TODO !
        }
    }

    /**
     * set output data dimensions and link connected wire
     * Note: actual data is stored always in workspace! Pin and wire are linked to WS var
     */
    void Mux::discreteOutputsInit(ElementBase::Pin *inheritFrom){
        size_t out_size = 0;
        for (auto& pin:inputs){
            if (pin->isConnected()){
                out_size += pin->getConnectedMarker()->getWire()->getDataSize();
            }else{
                out_size += 1;
            }
        }
        // init out
        initPortSize(outPin, out_size);

        // create cache
        dataCache.resize(out_size);
        out_size = 0;
        for (auto& pin:inputs){
            if (pin->isConnected()){
                auto pin_data_size = pin->getConnectedMarker()->getWire()->getDataSize();
                for (size_t i=0; i<pin_data_size; i++){
                    dataCache[out_size] = pin->data+i;
                    out_size++;
                }
            }else{
                dataCache[out_size] = &defaultVal;
                out_size += 1;
            }
        }
    }

    void Mux::discreteStep(double time){
        (void)time;

        double *wsData = outVar->getRef();
        for(auto i=dataCache.size(); i-->0;){
            wsData[i] = *dataCache[i];
        }
    }

}
