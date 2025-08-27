#include "DiscreteSys.h"
#include "../ElementBase/Element.h"


using namespace ElementBase;
using namespace MathPack;

namespace dae_solver{
    void verifySizeCompatible(Pin* p,
                            Connections::WireCluster *wire,
                            WorkSpace::Variable *wsVar,
                            Element *elem)
    {
        Q_UNUSED(wsVar);
        if (p->dataSizeSetting == size_t(-1)){
            // inherited
            // just accept wire size (?)
        }else{
            if (wire->getDataSize() != p->dataSizeSetting){
                throw runtime_error("Source of pin:"+p->getName()+" elem:"+elem->getName()+" has incompatible size!\n"
                                    "Has:"+to_string(wire->getDataSize())+"\n"
                                    "Expect:"+to_string(p->dataSizeSetting));
            }
        }
        
    }

    struct Chain{
        vector<Element*> chain;
        vector<bool> chainBreaks;
    };

    /**
     * Create sequence chain of execution (recursively)
     */
    list<Element*> createExecutionChain(
                                            Element* baseElem,
                                            vector<Element*> const& elemList,
                                            Chain *elemChain = nullptr
                                        )
    {
        bool deleteChain = elemChain == nullptr;
        if(deleteChain)
            elemChain = new Chain();

        list<Element*> ret;

        // check for loops
        for (size_t i=0; i<elemChain->chain.size(); i++){
            if (elemChain->chain[i] == baseElem){
                // possible loop detected; check break points
                bool isAlgLoop = true;
                for (size_t j=i; j<elemChain->chainBreaks.size(); j++){
                    // if has at least one break - it's not a loop
                    if (elemChain->chainBreaks[i]){
                        isAlgLoop = false;
                        break;
                    }
                }
                if (isAlgLoop)
                    throw runtime_error("Element "+baseElem->getName() + " is in alg loop!");
                
                // not a loop; just abort (beacuse already processed)
                return ret;
            }
        }

        // put base
        ret.push_front(baseElem);

        // append sequence (scedule)
        for (auto& pin:baseElem->_getPins()){
            if (pin->isConnected() && pin->getDirection() == Domains::Input){
                // find source marker
                Element *srcElem = nullptr;
                for(auto& mrkr:pin->getConnectedMarker()->getWire()->getMarkers()){
                    if (mrkr->getDirection() == Domains::Output && mrkr->isPlugged()){
                        srcElem = mrkr->getConnectedPin()->getOwner();
                        break;
                    }
                }

                // if something is connected to input pin
                if (srcElem != nullptr){
                    // check if already was processed
                    if (contains(ret, srcElem)) 
                        continue;

                    // recursion into src elem
                    elemChain->chain.push_back(baseElem);
                    elemChain->chainBreaks.push_back(pin->is_cached);
                    auto srcChain = createExecutionChain(srcElem, elemList, elemChain);
                    // append src elements into the sequence
                    ret.splice(ret.begin(), srcChain);

                    elemChain->chain.pop_back();
                    elemChain->chainBreaks.pop_back();
                }
            }
        }

        if (deleteChain)
            delete elemChain;

        return ret;
    }

    DiscreteSystem::DiscreteSystem(Subsystem *system, shared_ptr<WorkSpace::WorkSpace> workspace): workspace(workspace){
        vector<Element*> elemList;
        // filter discrete elements
        for (auto& e:system->getElements(true)){
            if(e->isDiscrete()){
                elemList.push_back(e);
            }
        }

        // create execution chain
        executionSequence.clear();
        for(auto i=elemList.size(); i-->0;){
            // check is exists in the sequence
            if (contains(executionSequence, elemList[i]))
                continue; // try next

            vector<Element*> chain;
            vector<bool> chainBreaks;
            auto seq = createExecutionChain(elemList[i], elemList);
            for(auto& e:seq){
                if (!contains(executionSequence, e))
                    executionSequence.push_back(e);
            }
        }

        // qDebug
        qDebug() << "Disc Sys Sequence:";
        for(auto& e:executionSequence){
            qDebug() << e->getName();
        }
        qDebug() << Qt::endl;

        // init connections
        size_t INHERIT_SIZE = -1;
        for(auto& e:executionSequence){
            e->discreteOutputsInit();
        }
        // second run, for inputs
        for(auto& e:executionSequence){
            for(auto& p:e->_getPins()){
                if(p->getDirection() == Domains::Input){
                    if(!p->isConnected()){
                        // just connect to WS
                        const auto& wsVar = e->getWorkspace(false).get(p->getName());
                        if (p->dataSizeSetting == INHERIT_SIZE){
                            // set to size 1
                            wsVar->init({0.0});
                        }else{
                            wsVar->init(vector<double>(p->dataSizeSetting, 0.0));
                        }
                        p->data = wsVar->getRef();
                        p->dataSize = wsVar->getSize();
                    } else {
                        auto wire =  p->getConnectedMarker()->getWire();
                        auto srcLm = wire->findMarker(Domains::Output);
                        const auto& wsVar = e->getWorkspace(false).get(p->getName());
                        if (srcLm->isPlugged()){
                            // connected, verify compatibility
                            if(p->is_cached){
                                const auto& wsVar_cache = e->getWorkspace(false).get(p->getName()+PIN_CACHE_VAR_SUFFIX);
                                verifySizeCompatible(p.get(), wire, wsVar_cache, e);
                                
                                // link pin to workspace
                                p->dataSize = wsVar_cache->getSize();
                                p->data = wsVar_cache->getRef();
                            }else{
                                verifySizeCompatible(p.get(), wire, wsVar, e);
                                // bypass ws var?
                                // TODO link srd WS var with dst WS var
                            }
                        } else {
                            // connected wire has no source; treat as not connected pin
                            if (p->dataSizeSetting == INHERIT_SIZE){
                                // set to size 1
                                wsVar->init({0.0});
                            }else{
                                wsVar->init(vector<double>(p->dataSizeSetting, 0.0));
                            }
                            p->data = wsVar->getRef();
                            p->dataSize = wsVar->getSize();
                        }
                    }
                }
            }
        }

        for(auto& e:elemList){
            // collect continuous diffs
            auto varList = e->getWorkspace(false).getVarList();
            // gather all discrete diffs
            for (auto& var:varList){
                if(var->type() == WorkSpace::Differential && var->dataType == WorkSpace::Discrete){
                    auto dX = dynamic_cast<WorkSpace::DifferentialVar*>(var.get());
                    auto X = dX->baseVar;
                    

                    // verify init X matches out size
                    if (X->getSize() != dX->getInitVal()->getSize())
                        throw runtime_error(e->getName()+" initial value size doesn't match the output!");
                    // apply init val
                    X->setValue(dX->getInitVal()->getVectorValue());
                    // init dX
                    // Note: not 100% correct, but ok because will be overriden by continuousStep
                    dX->init(dX->getInitVal()->getVectorValue());
                    double *X_ref = dynamic_cast<WorkSpace::DifferentialVar*>(var.get())->baseVar->getRef(),
                            *dX_ref=var->getRef();

                    // link to global state lists
                    for(size_t i=0; i<var->getSize(); i++){
                        dXvector.push_back(dX_ref+i);
                        Xvector.push_back(X_ref+i);
                    }
                }
            }
        }
        
    }

    void DiscreteSystem::init(){
        for(auto& e:executionSequence){
            e->discreteInit();
        }
    }

    void DiscreteSystem::discreteStep(double time){
        for(auto& e:executionSequence){
            e->discreteStep(time);
        }
    }

    void DiscreteSystem::continuousStep(){
        for(auto& e:executionSequence){
            e->continuousStep();
        }
    }
}
