#include "Outport.h"

using namespace ElementBase;

namespace Elements{
    Outport::Outport(){
        id = "Outport";
        name = "Outport";
        imgPath = ":/src/data/Elements/Discrete/Environment/Outport.png";

        out = addPin("ext", QPointF(0, 0), Domains::MATH.get(), true, Domains::ConnDirection::Output);
        in = addPin("inn", QPointF(0, 15), Domains::MATH.get(), false, Domains::ConnDirection::Input);

        description = "Passes values from and in sybsystems";

        // combobox param
        vector<string> titles, names;
        for (auto& dom:Domains::Domain::getDomains()){
            if (dom->directional) continue;
            
            titles.push_back(dom->typeTitle);
            names.push_back(dom->typeName);
        }
        parameters.emplace_back(make_unique<ComboBoxParam>("Domain", "Domain", Domains::ELECTRIC->typeName, titles, names));
    }

    ElementBase::Pin* Outport::getOuter() {
        return pins[0].get();
    }

    ElementBase::Pin* Outport::getInner() {
        return pins[1].get();
    }

    void Outport::discreteStep(double time){
        std::memcpy(out->data, in->data, out->dataSize*sizeof(double));
    }

}
