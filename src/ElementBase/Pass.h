#ifndef EBASE_PASS_H
#define EBASE_PASS_H

namespace ElementBase{
    class Pin;
}

//#include "Element.h"

namespace Elements{
    class Pass{
        public:
            virtual ElementBase::Pin* getOuter() = 0;
            virtual ElementBase::Pin* getInner() = 0;
    };
}

#endif
