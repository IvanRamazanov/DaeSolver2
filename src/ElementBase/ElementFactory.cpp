#include "ElementFactory.h"

#include "Subsystem/Subsystem.h"
#include "../Elements/Physics/Environment/Port/Port.h"
#include "../Elements/Discrete/Environment/Inport/Inport.h"
#include "../Elements/Discrete/Environment/Outport/Outport.h"
#include "../Elements/Discrete/Environment/Mux/Mux.h"
#include "../Elements/Discrete/Environment/Demux/Demux.h"
#include "../Elements/Discrete/Display/Scope/Scope.h"
#include "../Elements/Discrete/Display/XYGraph/XYGraph.h"
#include "../Elements/Discrete/Sources/Sinus/Sinus.h"
#include "../Elements/Discrete/Sources/Constant/Constant.h"
#include "../Elements/Discrete/Sources/Step/Step.h"
#include "../Elements/Discrete/Sources/Ramp/Ramp.h"
#include "../Elements/Discrete/Sources/SimulationTime/SimulationTime.h"
#include "../Elements/Discrete/Sources/SqeezeWave/SqeezeWave.h"
#include "../Elements/Discrete/Continuous/Delay/Delay.h"
#include "../Elements/Discrete/Continuous/Integrator/Integrator.h"
#include "../Elements/Discrete/Continuous/TransferFunction/TransferFunction.h"
#include "../Elements/Discrete/Basics/Gain/Gain.h"
#include "../Elements/Discrete/Basics/Product/Product.h"
#include "../Elements/Discrete/Basics/Saturation/Saturation.h"
#include "../Elements/Discrete/Basics/Summ/Summ.h"
#include "../Elements/Discrete/Control/ABCtoAB0/ABCtoAB0.h"
#include "../Elements/Discrete/Control/ABCtoDQ/ABCtoDQ.h"
#include "../Elements/Discrete/Control/Decompose/Decompose.h"
#include "../Elements/Discrete/Control/PIDController/PIDController.h"
#include "../Elements/Discrete/Control/RMSvalue/RMSvalue.h"
#include "../Elements/Discrete/Control/SwitchPath/SwitchPath.h"
#include "../Elements/Physics/Electric/Measurements/Ampermeter/Ampermeter.h"
#include "../Elements/Physics/Electric/Measurements/Voltmeter/Voltmeter.h"
#include "../Elements/Physics/Electric/Measurements/ThreePhMultimeter/ThreePhMultimeter.h"
#include "../Elements/Physics/Electric/ThreePhase/Measurements/Multimeter/Multimeter.h"
#include "../Elements/Physics/Rotational/Measurements/SpeedSensor/SpeedSensor.h"
#include "../Elements/Physics/Rotational/Measurements/TorqueSensor/TorqueSensor.h"


using namespace Elements;
using Factory = std::function<ElementBase::Element*()>;

namespace ElementBase{
    Element* makeElement(string const& id) noexcept {
        Element *ret;

        static const std::unordered_map<std::string, Factory> factories = {
            {SYS_ID,   [] { return new Subsystem(); }},
            {"Port",     [] { return new Port(); }},
            {"Inport",   [] { return new Inport(); }},
            {"Outport",  [] { return new Outport(); }},
            {"Sinus",    [] { return new Sinus(); }},
            {"Scope",    [] { return new Scope(); }},
            {"XYGraph",    [] { return new XYGraph(); }},
            {"Delay",    [] { return new Delay(); }},
            {"Ampermeter", [] { return new Ampermeter(); }},
            {"Voltmeter",  [] { return new Voltmeter(); }},
            {"Mux", [] { return new Mux();}},
            {"Demux", [] { return new Demux();}},
            {"Integrator",    [] { return new Integrator(); }},
            {"ThreePhMultimeter",    [] { return new ThreePhMultimeter(); }},
            {"SpeedSensor",    [] { return new SpeedSensor(); }},
            {"TorqueSensor",    [] { return new TorqueSensor(); }},
            {"Multimeter",    [] { return new Multimeter(); }},
            {"Gain",    [] { return new Gain(); }},
            {"Product",    [] { return new Product(); }},
            {"Saturation",    [] { return new Saturation(); }},
            {"Sum",    [] { return new Sum(); }},
            {"Constant",    [] { return new Constant(); }},
            {"Step",    [] { return new Step(); }},
            {"Ramp",    [] { return new Ramp(); }},
            {"SimulationTime",    [] { return new SimulationTime(); }},
            {"SqeezeWave",    [] { return new SqeezeWave(); }},
            {"TransferFunction",    [] { return new TransferFunction(); }},
            {"ABCtoAB0",    [] { return new ABCtoAB0(); }},
            {"ABCtoDQ",    [] { return new ABCtoDQ(); }},
            {"Decompose",    [] { return new Decompose(); }},
            {"PIDController",    [] { return new PIDController(); }},
            {"RMSvalue",    [] { return new RMSvalue(); }},
            {"SwitchPath",    [] { return new SwitchPath(); }},
        };

        if (auto it = factories.find(id); it != factories.end()) {
            ret = it->second();
        }else if(id.at(0) == ':'){
            // internal lib
            ret = new Element(id);
        }else if(XmlParser::startsWith(id, "Custom/")){
            // TODO custom library
            cerr << "Custom libs are not yet implemented" << endl;
            return nullptr; // or throw ?
        }else{
            // none matched
            cerr << "Unknown element: " + id << endl;
            return nullptr; // or throw ?
        }
        
        // init type flags
        ret->initFlags();
        ret->initView();
        return ret;
    }

    Element* makeElement(XmlParser::XmlElement *elemInfo) noexcept {
        if (elemInfo->attributes.count(ATTR_ID) == 0){
            cerr << "Element ID is missing: " + elemInfo->name << endl;
            return nullptr;
        }
        
        auto ret = makeElement(elemInfo->attributes[ATTR_ID]);
        if (ret != nullptr)
            ret->loadState(elemInfo);
        return ret;
    }
}
