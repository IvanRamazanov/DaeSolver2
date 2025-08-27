#ifndef ELEMS_PORT_H
#define ELEMS_PORT_H

namespace ElementBase{
    class Element;
}
namespace Elements{
    class Pass;
}

#include "../../../../ElementBase/Element.h"
#include "../../../../ElementBase/Pass.h"

namespace Elements{
    class Port : public ElementBase::Element, virtual public Pass {
        public:
            Port();

            void loadState(XmlParser::XmlElement *elemInfo) override;

            ElementBase::Pin* getOuter() override;

            ElementBase::Pin* getInner() override;

    };
}

#endif
