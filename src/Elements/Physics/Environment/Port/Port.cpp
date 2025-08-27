#include "Port.h"

using namespace ElementBase;

namespace Elements{
    Port::Port(){
        id = "Port";
        name = "Port";
        imgPath = ":/src/data/Elements/Physics/Environment/Port.png";
        description = "Passes values from and in subsystems";

        addPin("ext", QPointF(0, 0), Domains::ELECTRIC.get(), true);
        addPin("inn", QPointF(30, 15), Domains::ELECTRIC.get());

        equations.push_back(string("ext.p - inn.p"));

        // flow
        flows.push_back({pins[0].get(), pins[1].get()});

        // combobox param
        vector<string> titles, names;
        for (auto& dom:Domains::Domain::getDomains()){
            if (dom->directional) continue;
            
            titles.push_back(dom->typeTitle);
            names.push_back(dom->typeName);
        }
        parameters.push_back(make_unique<ComboBoxParam>("Domain", "Domain", Domains::ELECTRIC->typeName, titles, names));
    }

    void Port::loadState(XmlParser::XmlElement *elemInfo){
        // TODO base function will do just fine?

        if(!elemInfo->attributes.count(ATTR_ELEM_NAME))
            throw runtime_error("Port Element::Name attribute is expected");
        name = elemInfo->attributes[ATTR_ELEM_NAME];

        // layout
        auto node = elemInfo->find(ELEM_LAYOUT_TAG);
        if(node){
            auto layout = XmlParser::split(node->text, ",");
        }else{
            x = 0;
            y = 0;
            rot = 0;
        }

        // param
        string domain = Domains::ELECTRIC->typeName; // default
        node = elemInfo->find(ELEM_PARAMS_TAG);
        if (node){
            auto params = node->findAll(PARAM_TAG);
            for (auto& p:params){
                if(p->attributes.count(ATTR_PARAM_TYPE) && stoi(p->attributes[ATTR_PARAM_TYPE]) == ParamType::ComboBox){
                    parameters[0]->setValue(p->text);
                    break;
                }
            }
        }
    }

    ElementBase::Pin* Port::getOuter() {
        return pins[0].get();
    }

    ElementBase::Pin* Port::getInner() {
        return pins[1].get();
    }

}
