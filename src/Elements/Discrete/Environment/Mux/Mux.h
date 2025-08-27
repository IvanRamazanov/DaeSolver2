#ifndef ELEMS_MUX_H
#define ELEMS_MUX_H

#include "../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../ElementBase/Element.h"

namespace Elements{
    class Mux : public ElementBase::Element {
        ElementBase::Parameter *numOfInputs;
        vector<ElementBase::Pin*> inputs;
        ElementBase::Pin *outPin;
        WorkSpace::Variable *outVar = nullptr;
        double defaultVal = 0;
        vector<double*> dataCache;

        void updatePorts();

        public:
            Mux();

            void discreteStep(double time) override;
            void discreteOutputsInit(ElementBase::Pin *inheritFrom=nullptr) override;
    };
}

#endif
