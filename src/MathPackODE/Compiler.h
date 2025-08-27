#ifndef MATHODE_COMPILER_H
#define MATHODE_COMPILER_H

#include <vector>
#include <string>
#include "../ElementBase/ElemBaseDecl.h"
#include "DAE.h"
#include "DiscreteSys.h"

using namespace std;

namespace dae_solver {

    /**
     *
     * @author Ivan
     */
    class Compiler {
        public:
            bool recompile = true;
            shared_ptr<DAE> DAEsys;
            shared_ptr<DiscreteSystem> discSys;
            shared_ptr<WorkSpace::WorkSpace> workspace;

        public:
            Compiler(ElementBase::Subsystem *state, shared_ptr<dae_solver::Logger> logger);
    };
}

#endif
