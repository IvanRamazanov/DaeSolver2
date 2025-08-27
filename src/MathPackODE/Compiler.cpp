#include "Compiler.h"

using namespace std;
using namespace ElementBase;

namespace dae_solver{

    Compiler::Compiler(Subsystem *state, shared_ptr<Logger> logger){
        vector<Element*> physElemList; // must be ordered!
        vector<Element*> discreteElemList;
        vector<Connections::WireCluster*> wireList;
        workspace = make_shared<WorkSpace::WorkSpace>();
        // add global vars/consts
        workspace->add(WorkSpace::TIME_VAR_NAME, 0); // TODO T_start, not just 0?
        workspace->add("pi", 3.141592653589793, true);

        for(auto& e:state->getElements(true)){
            if (e->isPhysical())
                physElemList.push_back(e);
            if (e->isDiscrete())
                discreteElemList.push_back(e);
        }
        
        for (auto& w:state->getWires()){
            if(!w->getDomain()->directional)
                // gather only physical wires
                wireList.push_back(w);
        }

        // create physics and discrete systems
        if((wireList.empty()||physElemList.empty()) && discreteElemList.empty()){
            throw runtime_error("Empty Scheme");
        }

        DAEsys = make_shared<DAE>(physElemList, workspace, logger);
        //DAEsys.initJacobian(state.getJacobianEstimationType());
        DAEsys->initJacobian(2); // symbolic Jac only (?)

        // discrete system
        discSys = make_shared<DiscreteSystem>(state, workspace);
        
    }
}
