#ifndef ELEMS_DEMUX_H
#define ELEMS_DEMUX_H

#include "../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../ElementBase/Element.h"

namespace Elements{
    class Demux : public ElementBase::Element {
        ElementBase::Parameter *numOfOutputs;
        vector<ElementBase::Pin*> outputs;
        ElementBase::Pin *inPin;

        void updatePorts();

        public:
            Demux();

            void discreteStep(double time) override;
            void discreteInit() override;
    };
}

#endif
