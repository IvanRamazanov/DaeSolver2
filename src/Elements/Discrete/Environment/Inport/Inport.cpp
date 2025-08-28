#include "Inport.h"

using namespace ElementBase;

namespace Elements{
    Inport::Inport(){
        id = "Inport";
        name = "Inport";
        setImgPath(":/src/data/Elements/Discrete/Environment/Inport.png");

        in = addPin("ext", QPointF(0, 0), Domains::MATH.get(), true, Domains::ConnDirection::Input);
        out = addPin("inn", QPointF(30, 15), Domains::MATH.get(), false, Domains::ConnDirection::Output);

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

    ElementBase::Pin* Inport::getOuter() {
        return pins[0].get();
    }

    ElementBase::Pin* Inport::getInner() {
        return pins[1].get();
    }

    void Inport::discreteStep(double time){
        (void)time;
        std::memcpy(out->data, in->data, out->dataSize*sizeof(double));
    }
}
