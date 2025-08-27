#ifndef MATHODE_DISCR_SYS_H
#define MATHODE_DISCR_SYS_H

#include <vector>
#include <memory>
#include "../ElementBase/ElemBaseDecl.h"
#include "../MathPack/WorkSpace.h"

namespace dae_solver {
    class DiscreteSystem {
        vector<ElementBase::Element*> executionSequence;
        shared_ptr<WorkSpace::WorkSpace> workspace;

        public:
            vector<double*> Xvector, dXvector; // continuous diffs (not Z^-1)

            
        public:
            DiscreteSystem(ElementBase::Subsystem *system, shared_ptr<WorkSpace::WorkSpace> workspace);

            void init();

            void discreteStep(double time);

            // Mainly for updating dX(?)
            void continuousStep();
    };
}

#endif
