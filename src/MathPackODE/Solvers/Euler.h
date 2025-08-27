#ifndef SOLVERS_EULER_H
#define SOLVERS_EULER_H

#include "Solver.h"

namespace Solvers{
    class Euler : public Solver {
        public:
            Euler();

            void evalNextStep() override;
    };
}

#endif
