#ifndef ELEMS_VOLTMETER_H
#define ELEMS_VOLTMETER_H

#include "../../../../../ElementBase/ElemBaseDecl.h"
#include "../../../../../ElementBase/Element.h"

namespace Elements{
    class Voltmeter : public ElementBase::Element {
        WorkSpace::Variable *outVar, // output
                            *p1Var, // observable var
                            *p2Var; 

        public:
            Voltmeter();
            
            void discreteStep(double time) override;
    };
}

#endif
