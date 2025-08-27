#include "StringFunctionSystem.h"
#include "../MathPack/MatrixEqu.h"

using namespace std;
using Element = ElementBase::Element;
using Pin = ElementBase::Pin;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;
using namespace MathPack::MatrixEqu;

namespace MathPack{
    vector<Pin*> getPins(Element* elem, Domain *dom, Direction dir=Direction::Uni){
        vector<Pin*> ret;
        for (auto& p:elem->_getPins()){
            if (p->getDomain() == dom && p->getDirection() == dir){
                ret.push_back(p.get());
            }
        }
        return ret;
    }



    /**
     * @param pinCounter - counter to keep track of position in workspace vector (flows & potentials)
     */
    StringFunctionSystem::StringFunctionSystem(ElementBase::Element *element, WorkSpace::WorkSpace *ws){
        // sort variables
        vector<shared_ptr<WorkSpace::Variable>> elemVars = element->getWorkspace().getVarList();
        for (auto& var:elemVars){
            if(var->type() != WorkSpace::Differential){
                // TODO: filter unused vars. For e.g. not all local vars may be used in equations
                if (var->dataType == WorkSpace::Flow){
                    flows.push_back(var);
                }else if (var->dataType == WorkSpace::Potential){
                    potns.push_back(var);
                }
            }else{
                if (var->dataType != WorkSpace::Discrete){
                    diffVars.push_back(dynamic_pointer_cast<WorkSpace::DifferentialVar>(var));
                }
            }
        }
        elemVars.insert(elemVars.end(), ws->getVarList().begin(), ws->getVarList().end());

        // process Equations
        for(auto& str:element->getEquations()){
            // clone
            auto equation = make_shared<StringGraph>(str);
            // link SG vars to WS vars
            vector<string> equVars = equation->getVariableNames();
            // verify that all vars are known (could be linked to ws)
            for (auto& var:equVars){
                // try to find var in Element's workspace first
                bool found = false;
                for (auto& wsVar:elemVars){
                    if (wsVar->getName() == var){
                        found = true;
                        break;
                    }
                }
                // verify that var was found
                if (!found){
                    throw runtime_error("Unknown variable: " + var + " in elem:"+element->getName());
                }
            }
            // link equations to ws vars
            for (auto& var:elemVars){
                // workspace var was found, link it to StringGraph
                if(var->isConst){
                    equation->replaceWithConst(var->getName(), var->getValue());
                }else if(var->type() == WorkSpace::WsTypes::Differential){
                    // process Diffs only if thtey are present in equation
                    bool found = false;
                    for (auto& eqVar:equVars){
                        if (var->getName() == eqVar){
                            found = true;
                            break;
                        }
                    }
                    if (found){
                        equation->linkVariableToWorkSpace(var->getName(), var.get());
                    }
                    
                }else{
                    // replace (?) with var linked to WS
                    equation->linkVariableToWorkSpace(var->getName(), var.get());
                }
            }

            // verify that evrything is replaced!
            eqSystem.push_back(equation);
        }
    }

    StringFunctionSystem::StringFunctionSystem(vector<StringFunctionSystem> funcList){
        // TODO Log settings?
        for(auto& func:funcList){
            append(flows, func.flows);
            append(potns, func.potns);
            append(diffVars, func.diffVars);
            append(eqSystem, func.eqSystem);
        }
    }

    /**
     * String to int. if brackets are present, "[]" converts string before brackets
     */
    int StringFunctionSystem::parseInt(string str){
        // TODO why not just stoi?
        int i;
        if((i=XmlParser::indexOf(str, "["))!=-1){
            string numb = str.substr(0, i);
            return stoi(numb);
        }else {
            return stoi(str);
        }
    }

    void StringFunctionSystem::renameVariable(string const& oldName, string const& newName){
        for (auto& func:eqSystem){
            func->renameVariable(oldName, newName);
        }
    }

    void StringFunctionSystem::linkVarToWS(string const& name, WorkSpace::Variable *link){
        for (auto& func:eqSystem){
            func->linkVariableToWorkSpace(name, link);
        }
    }

}
