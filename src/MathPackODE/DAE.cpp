/*
 * The MIT License
 *
 * Copyright 2017 Ivan.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "DAE.h"
#include "../ElementBase/ElemBaseDecl.h"
#include "../MathPack/MatrixEqu.h"

using namespace std;
using namespace MathPack::MatrixEqu;
using namespace ElementBase;
using namespace MathPack;
using namespace Connections;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;

namespace dae_solver {
    inline size_t getGlobalIdx(string const& localName,
                            vector<string> const& globalNames)
    {
        size_t ret = 0;
        for (auto& name:globalNames){
            if (name == localName){
                return ret;
            }
            ret++;
        }

        throw runtime_error("Can't find variable: "+localName);
    }

    /**
     * Create DAE system of equations: f(state, X, dX)
     * 
     * stateVector: element dynamic vars + element local dynamic vars (used for algebraic system solver)
     * 
     * stateVector ---> StringGraph 
     *              \-> elem WorkSpace (to enable hybrid elements; dynamic + discrete logic)
     * 
     * xVector -------> elem WorkSpace (any subset of stateVector; used by ODE solver)
     *             \--> StringGraph
     * 
     * dXVector ------> elem WorkSpace (any subset of element equation vars)
     *            \---> StringGraph
     * 
     * for alg solver, we exclude xVector from stateVector and append with dXVector.
     * updates into xVector will propagate into both equation system, and element WorkSpace
     * to achieve that, dXVector will be pointing into [0 : diffRank) portion of the stateVector
     * and element WorkSpace will be relinked mapping
     */
    DAE::DAE(vector<ElementBase::Element*> physElements,
            shared_ptr<WorkSpace::WorkSpace> ws,
            shared_ptr<dae_solver::Logger> logger): 
        logger(logger),    
        workspace(ws)
    {
        size_t physVectorSize = 0;
        vector<StringFunctionSystem> elemFuncs;

        // verify physical pins
        for(auto& elem:physElements){
            int i=0;
            for(auto& pin:elem->_getPins()){
                if(!pin->getDomain()->directional && !pin->isConnected())
                    throw runtime_error("All physical contacts must be connected! (Pin #"+to_string(i)+" in "+elem->getName());
                i++;
            }

            // count state vector size
            for(auto& v:elem->getWorkspace().getVarList()){
                // Note: techically, should be only vars used by equations
                if (v->dataType != WorkSpace::Discrete && v->type() == WorkSpace::Scalar){
                    physVectorSize += 1;
                }
            }
        }
        
        stateSize = physVectorSize; // initial length of state vector
        dynamicVars.resize(stateSize);
        dynDiffVars.resize(stateSize);
        stateToDynVars.resize(stateSize);

        // gather Elem equations
        size_t cntVar = 0;
        vector<string> globalVarNames(stateSize); // to get idx from connected pins (connection maps)
        logger->initLogs();
        for(auto& elem:physElements){
            StringFunctionSystem elemFunc(elem, ws.get());
            
            // make function var names unique
            string elemPath = elem->getPath();
            // Note: we assume that only dynamic vars that used in equations are present in varVec
            // which may not be true, when Workspace becomes editable by the user
            auto varVec = elemFunc.flows;
            append(varVec, elemFunc.potns);
            for (auto& wsVar:varVec){
                dynamicVars[cntVar] = wsVar;

                // new global (state) name
                string globalName = elemPath + "::" + wsVar->getName();
                elemFunc.renameVariable(wsVar->getName(), globalName);
                elemFunc.linkVarToWS(globalName, wsVar.get());

                // for connection mapping
                globalVarNames[cntVar] = globalName;

                // find corresponding diff var
                for (auto& dv:elemFunc.diffVars){
                    if (dv->baseVar == wsVar){
                        dynDiffVars[cntVar] = dv;

                        string globalName = elemPath + ElementBase::ELEM_PATH_DELIM + dv->getName();
                        elemFunc.renameVariable(dv->getName(), elemPath + "::" + dv->getName());
                        elemFunc.linkVarToWS(globalName, dv.get());

                        break;
                    }
                }
                
                // logging
                logger->printMsg(format("X{} = {}\n", cntVar++, globalName));
            }
            
            
            elemFuncs.push_back(elemFunc);
        }
        // gather
        StringFunctionSystem system(elemFuncs);
        algSystem = system.eqSystem;
        // for(auto& v:dynamicVars){
        //     workspace->add(v);
        // }
        // for(auto& v:dynDiffVars){
        //     workspace->add(v);
        // }
        
        // create connection maps for each doamin
        // list of domains present in current system
        vector<Domains::Domain*> systemDomains; 
        for(auto& elem:physElements){
            for(auto& pin:elem->_getPins()){
                if(pin->getDomain()->directional)
                    continue;
                    
                const auto& dom = pin->getDomain();
                // check is exists
                bool present=false;
                for (auto& d:systemDomains){
                    if(d == dom){
                        present = true;
                        break;
                    }
                }
                if(!present)
                    systemDomains.push_back(dom);
            }
        }
        unordered_map<Domain*, vector<vector<int>>> domainConnections_flow;
        unordered_map<Domain*, vector<vector<int>>> domainConnections_potn;
        for(auto& dom:systemDomains){
            domainConnections_potn[dom] = getConnectionMap(physElements, dom->typeId, globalVarNames, false);
            domainConnections_flow[dom] = getConnectionMap(physElements, dom->typeId, globalVarNames, true);
        }
        // connections log
        // print matrices header
        logger->printMsg("\n");
        vector<string> stateNames(dynamicVars.size());
        for(size_t i =0; i<stateNames.size(); i++)
            stateNames[i] = "X"+to_string(i);
        logger->printMatrixHeader(stateNames);
        #ifdef DEBUG
        for(auto& [dom, map]:domainConnections_potn){
            logger->printMsg(format("\nInitial data. Domain: {:s}\n", dom->typeId));
            int cols;
            if(map.empty())
                cols=0;
            else
                cols=map[0].size();
            logger->printMsg(format("Potentials: {} out of {}\n", map.size(), cols));
            for(size_t x=0; x<map.size(); x++){
                logger->printMatRow(map[x]);
                logger->printMsg(" = 0\n");
            }
            logger->printMsg("\n");

            auto& f_map = domainConnections_flow[dom];
            if(f_map.empty())
                cols=0;
            else
                cols=f_map[0].size();
            logger->printMsg(format("Currents: {} out of {}\n", f_map.size(), cols));
            for(size_t x=0; x<f_map.size(); x++){
                logger->printMatRow(f_map[x]);
                logger->printMsg(" = 0\n");
            }
            logger->printMsg("\nFunctions:\n");
            for(size_t x=0; x<algSystem.size(); x++){
                string right="0",
                        left=algSystem[x]->toString();
                logger->printMsg(format("{} = {}\n", left, right));
            }
            logger->printMsg("\n");
        }
        #endif

        // init
        // combine all domains and potns+flows into common Connection Map
        vector<vector<int>> connections;
        for (auto& [dom, map]:domainConnections_potn){
            connections.insert(connections.end(), map.begin(), map.end());
        }
        for (auto& [dom, map]:domainConnections_flow){
            connections.insert(connections.end(), map.begin(), map.end());
        }
        // logs
        logger->printMsg("\nConnection map\n");
        for (auto& row:connections){
            logger->printMatRow(row);
            logger->printMsg("\n");
        }

        if(!connections.empty()){
            // convert connection map into eq system; reduce size
            setVarsByKirhgof(connections, globalVarNames);
        }

        // qDebug
        #ifdef DEBUG
        logger->printMsg("\n\nWS:\n");
        for (auto& v:workspace->getVarList()){
            logger->printMsg(v->getName()+" address:"+to_string((size_t)v->getRef())+"\n");
        }
        for(size_t i=0; i<dynamicVars.size(); i++){
            logger->printMsg(dynamicVars[i]->getName()+" address:"+to_string((size_t)dynamicVars[i]->getRef())+"\n");
        }
        for(size_t i=0; i<dynDiffVars.size(); i++){
            logger->printMsg(dynDiffVars[i]->getName()+" address:"+to_string((size_t)dynDiffVars[i]->getRef())+"\n");
        }
        logger->printMsg("\nstate vec:\n");
        for (auto& v:stateVector){
            logger->printMsg("Var address:"+to_string((size_t)v)+" value:"+to_string(*v)+"\n");
        }
        logger->printMsg("\nX vector:\n");
        for (auto& v:xVector){
            logger->printMsg("Var address:"+to_string((size_t)v)+" value:"+to_string(*v)+"\n");
        }
        logger->printMsg("\ndX vector:\n");
        for (auto& v:dXVector){
            logger->printMsg("Var address:"+to_string((size_t)v)+" value:"+to_string(*v)+"\n");
        }
        #endif
    }

    /**
     * Note: Elem pins must be already linked to workspece vector!
     */
    vector<vector<int>> DAE::getConnectionMap(vector<ElementBase::Element*> Elems,
                                            string const& domainId,
                                            vector<string> const& globalNames,
                                            bool flowMap)
    {
        vector<Connections::WireCluster*> dom_wires;

        // count rows (pins)
        size_t rowLength = dynamicVars.size();
        size_t columnLength = 0;

        // count columns (connections/equations)
        for(auto& elem:Elems){
            for(auto& pin:elem->_getPins()){
                if(pin->getDomain()->typeId == domainId){
                    const auto &wire = pin->getConnectedMarker()->getWire();
                    // Note: assumption that connection is already verified (domain)
                    // check if already exists
                    bool exists = false;
                    for (auto& w:dom_wires)
                        if(w == wire){
                            exists = true;
                            break;
                        }
                    if (!exists){
                        if (flowMap){
                            dom_wires.push_back(wire);
                            columnLength += wire->getDomain()->varSize;
                        }else{
                            dom_wires.push_back(wire);
                            columnLength += wire->getDomain()->varSize*(wire->size()-1);
                        }
                    }
                }
            }
        }
        // if(flowMap){
        //     columnLength--; // ??
        // }

        // find domain references
        vector<GraphFlowData> references;
        for(auto& elem:Elems){
            for(auto& flowData:elem->getFlows()){
                if(flowData.dom->typeId == domainId && flowData.is_reference){
                    // size 1 means it's a reference
                    references.push_back(flowData);
                }
            }
        }
        if(!references.empty() && flowMap)
            columnLength++; // add equation for ref flows sum

        // construct connection map
        // init matrix
        vector<vector<int>> out;
        for(size_t i = columnLength; i-->0;){
            out.push_back(vector<int>(rowLength, 0));
        }
        // go through connections
        size_t row=0;
        for(auto& wire:dom_wires){
            vector<size_t> masterIdx;
            for (auto& mrkr:wire->getMarkers()) {
                // filter unplugged pins
                if (mrkr->isPlugged()){
                    if(flowMap){
                        // sum of flows in the node = 0
                        auto p = mrkr->getConnectedPin();
                        auto &p_conn = p->connector;
                        string elemPath = p->getOwner()->getPath();
                        size_t k = 0;
                        for (size_t j = 0; j<p_conn.varNames.size(); j++){
                            if (p_conn.varTypes[j] == WorkSpace::WsDataType::Flow){
                                // find pin var in global list
                                size_t wsIdx = getGlobalIdx(elemPath + "::" + p_conn.varNames[j], globalNames);
                            
                                if (row+j<columnLength){
                                    out[row+k][wsIdx] = 1;
                                    k++;
                                }
                            }
                        }
                    }else{
                        // potentials in the node are all equal
                        if (masterIdx.empty()){
                            // init master
                            auto p = mrkr->getConnectedPin();
                            string elemPath = p->getOwner()->getPath();
                            auto &p_conn = p->connector;
                            for (size_t i=0; i<p_conn.varNames.size(); i++){
                                if (p_conn.varTypes[i] == WorkSpace::WsDataType::Potential){
                                    masterIdx.push_back(getGlobalIdx(elemPath + "::" + p_conn.varNames[i], globalNames));
                                }
                            }
                        }else{
                            // set slave equal to master
                            auto p = mrkr->getConnectedPin();
                            string elemPath = p->getOwner()->getPath();
                            auto &p_conn = p->connector;
                            size_t j = 0; // separate counter, because connector has mora vars than idx vector
                            for (size_t i=0; i<p_conn.varNames.size(); i++){
                                if (p_conn.varTypes[i] == WorkSpace::WsDataType::Potential){
                                    size_t slaveIdx = getGlobalIdx(elemPath + "::" + p_conn.varNames[i], globalNames);
                                    out[row][masterIdx[j]] = 1;
                                    out[row][slaveIdx] = -1;
                                    row++;
                                    j++;
                                }
                            }
                        }
                    }
                }
            }
            if (flowMap){
                row += wire->getDomain()->varSize;
                if(row>=columnLength)
                    break;
            }
        }

        // reference
        if (flowMap){
            // in case of flow map, assume that all references are implicitly linked together: sum(flow)=0
            for (auto& flowData:references){
                for(auto& pin:flowData.pins){
                    string elemPath = pin->getOwner()->getPath();
                    auto p_conn = pin->connector;
                    for (size_t i=0; i<p_conn.varNames.size(); i++){
                        if (p_conn.varTypes[i] == WorkSpace::WsDataType::Flow){
                            size_t ind = getGlobalIdx(elemPath + "::" + p_conn.varNames[i], globalNames);
                            out[columnLength - 1][ind] = 1;
                        }
                    }
                }
            }

            // internal flows (technically, could be done in one run with references)
            for (auto& elem:Elems){
                for (auto& flowData:elem->getFlows()){
                    // check if it's a reference
                    if(!flowData.is_reference && flowData.dom->typeId == domainId){
                        vector<int> tmpRow(rowLength);
                        string elemPath = (*flowData.pins.begin())->getOwner()->getPath();
                        for (auto& vn:flowData.varNames){
                            size_t ind = getGlobalIdx(elemPath + "::" + vn, globalNames);
                            tmpRow[ind] = 1;
                        }
                        out.push_back(tmpRow);
                    }
                }
            }
        }else{
            // in case of potential map, simply add reference zero, to random position
            vector<Pin*> refPins;
            if(references.empty()){
                // assign random one (first) to 0
                for(auto& elem:Elems){
                    for (auto& pin:elem->_getPins()){
                        if (pin->getDomain()->typeId == domainId){
                            refPins.push_back(pin.get());
                            goto process_refs;
                        }
                    }
                }
            }else{
                for(auto& flowData:references){
                    for (auto& p:flowData.pins)
                        refPins.push_back(p);
                }
            }

            process_refs:
            for(auto& pin:refPins){
                auto p_conn = pin->connector;
                for (size_t i=0; i<p_conn.varNames.size(); i++){
                    if (p_conn.varTypes[i] == WorkSpace::WsDataType::Potential){
                        string elemPath = pin->getOwner()->getPath();
                        size_t ind = getGlobalIdx(elemPath + "::" + p_conn.varNames[i], globalNames);
                        
                        // add new row for each ref potential
                        out.push_back(vector<int>(rowLength));
                        out[row][ind]=1;
                        row++;
                    }
                }
            }
        }

        return(out);
    }

    void DAE::setVarsByKirhgof(vector<vector<int>> & connMap, vector<string> const& globalNames)
    {
        set<size_t> stateVecIdxs; // independent variables

        // minimize connection maps
        // init right side for connection map (all zeroes)
        // append connection map with state vector (diagonal matrix)
        size_t numInependentVars = stateSize - MatrixEqu::rank(connMap);
        size_t varIdx=0;
        auto tmpMap = connMap;
        vector<int> row(stateSize, 0);
        for (size_t k = numInependentVars; k-->0;){
            // find independent vars and append connection matrix
            for (; varIdx<stateSize; varIdx++){
                row[varIdx] = 1;
                tmpMap.insert(tmpMap.begin(), row);
                // check rank
                auto rnk = MatrixEqu::rank(tmpMap);
                if(stateSize - rnk < numInependentVars){
                    // rank increased, therefore independent var was found!
                    stateVecIdxs.insert(varIdx);
                    numInependentVars--;
                    break;
                }else{
                    // varIdx is dependent; revert connMap
                    row[varIdx] = 0;
                    tmpMap.erase(tmpMap.begin());
                }
            }
        }
        if(stateSize != MatrixEqu::rank(tmpMap))
            throw runtime_error("Failed to find independent vars in connection map!");
        
        // logging
        logger->printMsg("\nIndependent vars:");
        for (auto& v:stateVecIdxs){
            logger->printMsg(" X" + to_string(v));
        }
        logger->printMsg("\n");

        // replace vars with independent ones
        // Note: result of replacement is: stateToOriginal vector is created
        // ...
        auto diffVars = replaceVars(connMap, globalNames, stateVecIdxs);

        // logging
        varIdx=0;
        logger->printMsg("\nState dependencies:\n");
        for (auto& sg:stateToDynVars){
            logger->printMsg(format("X{} = {}\n", varIdx++, sg->toString()));
        }
        varIdx=0;
        logger->printMsg("\nState diff dependencies:\n");
        for (auto& [dv, sg]:stateToDynDiffs){
            for(auto& var:dynDiffVars){
                if(var.get() == dv)
                    logger->printMsg(format("d.X{} = {}\n", varIdx++, sg ? sg->toString() : "null"));
            }
        }
        logger->printMsg("\nState vector: ");
        for (auto& eq:stateVecIdxs){
            logger->printMsg(" X" + to_string(eq));
        }
        logger->printMsg("\nFinal functions:\n");
        for (auto& eq:algSystem){
            logger->printMsg(eq->toString() + "\n");
        }

        // init state vector
        stateSize = stateVecIdxs.size();
        stateVector = vector<double*>(stateSize);
        xVector = vector<double*>(diffRank);
        dXVector = vector<double*>(diffRank);
        varIdx = 0;
        for (auto stIdx:stateVecIdxs){
            stateVector[varIdx] = dynamicVars[stIdx]->getRef();
            varIdx++;
        }
        // init ODE 
        varIdx = 0;
        for (auto& idx: diffVars){
            // link dX to stateVector (which is linked to algSystem)
            dXVector[varIdx] = dynDiffVars[idx]->getRef();

            // find base ws var of this diff
            xVector[varIdx] = dynamicVars[idx]->getRef();

            // replace stateVector var with diff
            size_t i = 0;
            for (auto stIdx:stateVecIdxs){
                if (stIdx == idx){
                    stateVector[i] = dynDiffVars[idx]->getRef();
                    
                    // also update names
                    // TODO: just store global names (both diff and var) in private field!
                    auto path = XmlParser::split(globalNames[idx], ElementBase::ELEM_PATH_DELIM);
                    path.back() = dynDiffVars[idx]->getName();
                    stateVectorNames[i] = XmlParser::join(path, ElementBase::ELEM_PATH_DELIM);
                    break;
                }

                i++;
            }

            varIdx++;
        }

        // init ws values
        // set state values
        for (auto i=stateSize; i-->0;){
            *stateVector[i] = 1e-8; // set to eps from zero, to prevent div-by-zero
        }

        // set X values (initial conditions)
        varIdx = 0;
        for (auto& idx:diffVars){
            // if no init value param is associated, set 0 by default
            *xVector[varIdx] = dynDiffVars[idx]->getInitVal() ? dynDiffVars[idx]->getInitVal()->getScalarValue() : 0;

            varIdx++;
        }

        // set dX values
        for (auto i=diffRank; i-->0;){
            *dXVector[i] = 1e-8; // set to eps from zero, to prevent div-by-zero
        }
    }

    /**
        * @return the algSystem
        */
    vector<shared_ptr<StringGraph>> DAE::getAlgSystem() {
        return algSystem;
    }

    /**
     * Generate var replacement vector: stateVar[i] = f(refVars...)
     * @param connMap Kirhgoff connection map
     * @param refVars list of independent vars within connection map
     * @return vector of formulas: stateVar[i] = f(refVars...)
     */
    set<size_t> DAE::replaceVars(vector<vector<int>> & connMap,
                        vector<string> const& globalNames,
                        set<size_t> & stateVecIdxs)
    {
        // gather diff vars
        set<size_t> diffPresent; // list of diff vars that needs to be replaced
        for(auto& eq:algSystem){
            for(auto& v:eq->getVariables()){
                if (v->getWsLink()->type() == WorkSpace::Differential){
                    // get var's index
                    for (auto i=dynDiffVars.size(); i-->0;){
                        if (dynDiffVars[i].get() == v->getWsLink()){
                            diffPresent.insert(i);
                            break;
                        }
                    }
                    
                }
            }
        }
        // for(auto& eq:stateToDynVars){
        //     for(auto& v:eq->getVariables()){
        //         if (v->getWsLink()->type() == WorkSpace::Differential){
        //             // get var's index
        //             for (auto i=dynDiffVars.size(); i-->0;){
        //                 if (dynDiffVars[i].get() == v->getWsLink()){
        //                     diffPresent.insert(i);
        //                     break;
        //                 }
        //             }
        //         }
        //     }
        // }

        // check for constants
        for (auto i = connMap.size(); i-->0;){
            size_t entries = 0;
            size_t idx = 0;
            for (size_t j=stateSize; j-->0;){
                // keep it branchless
                entries += abs(connMap[i][j]);
                idx = j*abs(connMap[i][j]) + idx;
            }

            if (entries == 1){
                // replace vars with 0 const
                stateToDynVars[idx] = make_shared<StringGraph>(0.0);
                for (auto k = connMap.size(); k-->0;){
                    connMap[k][idx] = 0;
                }

                // remove row
                connMap.erase(connMap.begin() + i);

                // reset cycle
                i = connMap.size();
            }
        }

        // replace
        #ifdef DEBUG
        logger->printMsg("\n\n~~~Reduce to state vars~~~\n");
        #endif
        for (auto& rv:stateVecIdxs){
            if (stateToDynVars[rv] != nullptr)
                throw runtime_error(format("X{} supposed to be a var, but it's const 0!", rv));

            stateToDynVars[rv] = make_shared<StringGraph>(dynamicVars[rv].get(), globalNames[rv]);
        }
        while (true){
            bool madeChange = false;

            for(auto k=connMap.size(); k-->0;){
                auto& row = connMap[k];
                // check if all but one vars are available
                int notAvailable = 0, varCnt = 0;
                vector<size_t> availIdxs;
                size_t idx;
                for(size_t i=stateSize; i-->0;){
                    if (row[i] != 0){
                        if (stateToDynVars[i] == nullptr){
                            notAvailable++;
                            idx = i;
                        }else{
                            availIdxs.push_back(i);
                        }
                    }
                }
                // if only one, replace it
                if(notAvailable == 1){
                    // idx'th could be replaced with sum of available idxs
                    vector<shared_ptr<StringGraph>> inps(availIdxs.size());
                    vector<int> gains(availIdxs.size());
                    for (size_t j=availIdxs.size(); j-->0;){
                        gains[j] = row[idx] * row[availIdxs[j]] * -1;
                        inps[j] = stateToDynVars[availIdxs[j]];
                    }
                    stateToDynVars[idx] = StringGraph::sum(inps, gains);

                    #ifdef DEBUG
                    logger->printMsg("Replaced "+globalNames[idx]+" with "+stateToDynVars[idx]->toString()+"\n");
                    #endif

                    // zero "replaced" row
                    connMap.erase(connMap.begin() + k);
                    madeChange = true;
                    break;
                }else if(notAvailable == varCnt){
                    // already has all vars replaced
                    connMap.erase(connMap.begin() + k);
                }
            }

            if(!madeChange)
                break;
        }
        #ifdef DEBUG
        logger->printMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
        #endif

        // verify
        for(auto& row:connMap){
            for(auto& v:row){
                if (v != 0)
                    throw runtime_error("Wasn't able to replace all variables in connection matrix");
            }
        }
        // replace vars in equations
        // Note: a bit excessive? replace only vars that need to be replaced?
        for (auto& eq:algSystem){
            for (size_t i=0; i<stateSize; i++){
                eq->replaceVariable(globalNames[i], stateToDynVars[i]);
            }
        }

        // look for 'trivial' vars: depend only on discrete variables
        #ifdef DEBUG
        logger->printMsg("\n\n~~~Reduce trivial vars~~~\n");
        logger->printMsg("Alg system:\n");
        for (size_t i=0; i<algSystem.size();i++){
            logger->printMsg(algSystem[i]->toString() +" = 0\n");
        }
        #endif
        bool volatile exitLoop = false;
        while (!exitLoop){
            bool madeChange = false;

            // for(size_t i=algSystem.size(); i-->0;){
            //     auto varList = algSystem[i]->getVariables();
            //     vector<string> namesNonDiscrete;
            //     for(auto& v:varList){
            //         if (v->workSpaceLink->type() != WorkSpace::Discrete){
            //             namesNonDiscrete.push_back(v->getName());
            //         }
            //     }
            //     for(auto& vn:namesNonDiscrete){
            //         if(algSystem[i]->canGet(vn)){
            //             // vn - could be replaced
            //             algSystem[i]->getVariable(vn);
            //             size_t vnIdx = stoi(vn.substr(1));
            //             ret[vnIdx] = algSystem[i];
            //             for(size_t k=algSystem.size(); k-->0;){
            //                 if (k != i)
            //                     algSystem[k]->replaceVariable(vn, algSystem[i]);
            //             }

            //             madeChange = true;
            //             break;
            //         }
            //     }
            // }

            // version with only state vars (ignore local)
            for(auto stIdx=stateVecIdxs.begin(); stIdx!=stateVecIdxs.end(); ){
                bool increment = true;
                for(size_t i=algSystem.size(); i-->0;){
                    auto &refName = globalNames[*stIdx];
                    if(algSystem[i]->canGet(refName)){
                        #ifdef DEBUG
                        logger->printMsg("Getting "+refName+" from "+algSystem[i]->toString()+"\n");
                        #endif

                        // can get ref
                        algSystem[i]->getVariable(refName);
                        stateToDynVars[*stIdx] = algSystem[i];
                        // ref is no longer a ref; clean up
                        algSystem.erase(algSystem.begin() + i);

                        // replace in other equations
                        for (auto& eq:algSystem){
                            eq->replaceVariable(refName, stateToDynVars[*stIdx]);
                        }
                        for (auto& eq:stateToDynVars){
                            eq->replaceVariable(refName, stateToDynVars[*stIdx]);
                        }

                        #ifdef DEBUG
                        logger->printMsg("Replaced "+refName+" with "+stateToDynVars[*stIdx]->toString()+"\n");
                        #endif

                        // ref is no longer a ref; clean up
                        stIdx = stateVecIdxs.erase(stIdx);
                        increment = false;

                        madeChange = true;
                        break;
                    }
                }

                if(stateVecIdxs.size() == 1){
                    // leave at least one var
                    exitLoop = true;
                    break;
                }
                
                if(increment)
                    stIdx++;
            }

            if(!madeChange)
                exitLoop = true;
        }
        #ifdef DEBUG
        logger->printMsg("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
        #endif

        #ifdef DEBUG
        logger->printMsg("\n\n~~~Before init conditions~~~\n");
        logger->printMsg("State vec:\n");
        for (size_t i=0; i<stateToDynVars.size();i++){
            logger->printMsg(globalNames[i] +" = "+stateToDynVars[i]->toString()+"\n");
        }
        logger->printMsg("\nAlg system:\n");
        for (size_t i=0; i<algSystem.size();i++){
            logger->printMsg(algSystem[i]->toString() +" = 0\n");
        }
        logger->printMsg("\nIndependent vars: ");
        for (auto& v:stateVecIdxs){
            logger->printMsg("X"+to_string(v)+" ");
        }
        logger->printMsg("\nDiffs present: ");
        for (auto& v:diffPresent){
            logger->printMsg("X"+to_string(v)+" ");
        }
        logger->printMsg("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
        #endif

        // process diff initial conditions
        // treat X var as discrete and replace state vars with it
        for (auto& diffIdx:diffPresent){
            // check if independent
            auto varList = stateToDynVars[diffIdx]->getVariables();

            bool isIndependent=false;
            for(auto& var:varList){
                // independent, if has at least one dynamic var
                auto idx = indexOf(dynamicVars, var->getWsLink());
                if (idx!=size_t(-1)
                    && var->getWsLink()->dataType != WorkSpace::Discrete
                    && !diffPresent.contains(idx))
                {
                    isIndependent = true;

                    auto left = make_shared<StringGraph>(dynamicVars[diffIdx].get(), globalNames[diffIdx]);
                    #ifdef DEBUG
                    logger->printMsg("Getting "+globalNames[idx]+" from "+stateToDynVars[diffIdx]->toString()+"="+left->toString()+"\n");
                    #endif
                    stateToDynVars[diffIdx]->getVariable(globalNames[idx], left);
                    #ifdef DEBUG
                    logger->printMsg("Result: "+globalNames[idx]+" = "+stateToDynVars[diffIdx]->toString()+"\n");
                    #endif

                    // replace
                    #ifdef DEBUG
                    logger->printMsg("Replaced "+globalNames[idx]+" with "+stateToDynVars[diffIdx]->toString()+"\n");
                    #endif
                    for(auto& eq:algSystem){
                        eq->replaceVariable(globalNames[idx], stateToDynVars[diffIdx]);
                    }
                    for(auto& eq:stateToDynVars){
                        eq->replaceVariable(globalNames[idx], stateToDynVars[diffIdx]);
                    }

                    // no longer a state var
                    for(auto stIdx:stateVecIdxs){
                        if(stIdx == idx){
                            isIndependent = false; // just to mark that change was made
                            stateVecIdxs.erase(stIdx);
                            break;
                        }
                    }
                    if(isIndependent)
                        throw runtime_error("Strange, non state variable is still present: "+globalNames[idx]);

                    // but dynamicVars[diffIdx] now is
                    stateVecIdxs.insert(diffIdx);
                    stateToDynVars[diffIdx] = left;

                    isIndependent = true;
                    break;
                }
            }

            if(!isIndependent){
                // check if all discrete
                bool allDiscrete=true;
                for (auto& var:varList){
                    if(WorkSpace::WorkSpace::isDynamicVariable(var->getWsLink())){
                        allDiscrete = false;
                        break;
                    }
                }
                if (allDiscrete){
                    ostringstream oss;
                    oss << "Differential: "<<dynDiffVars[diffIdx]->getName() << " is in ";
                    if (dynDiffVars[diffIdx]->dataType == WorkSpace::Flow)
                        oss<< "series";
                    else
                        oss<< "parallel";
                    oss<< " to a source element!";
                    throw runtime_error(oss.str());
                }
            }
        }
        // resolve possible init val 'conflicts'
        set<size_t> diffInitValues; // holds idxes of set diffs; to keep track of non-inited diffs
        // Cycle through max to min priority
        vector<size_t> tmpVec(diffPresent.begin(), diffPresent.end()); // for erasing more conveniently
        qDebug() << "Init priority cycle...";
        for (int i=1; i>=0; i--){
            qDebug() << "Prior:" << i;
            for (auto k=tmpVec.size(); k-->0;){
                auto diffIdx = tmpVec[k];
                auto initVal = dynDiffVars[diffIdx]->getInitVal();
                if (initVal != nullptr && (int)initVal->getPriority() == i){
                    qDebug() << "Diff Var: " << globalNames[diffIdx] << " has prior:" << (int)initVal->getPriority() << " value:"<<initVal->getScalarValue();

                    // check if it's a state var
                    bool isPartOfState = false;
                    for (auto& stIdx:stateVecIdxs){
                        if (stIdx == diffIdx){
                            // part of the state vec. 
                            isPartOfState = true;

                            // Just mark as processed
                            diffInitValues.insert(diffIdx);

                            qDebug() << "Do nothing...";

                            break;
                        }
                    }

                    if(!isPartOfState){
                        qDebug() << "Try to replace...";
                        // not part of the state. Try to return it back to state vector

                        // check if can replace one of the state vars
                        auto varList = stateToDynVars[diffIdx]->getVariables();
                        size_t canReplace = -1;
                        for(auto& var:varList){
                            // get var index
                            for(auto j=dynamicVars.size(); j-->0;){
                                if (var->getWsLink() == dynamicVars[j].get()){
                                    if(diffInitValues.contains(j)){
                                        // was already processed, therefore can't be replaced
                                        break;
                                    }

                                    // wasn't processed; could be replaced
                                    canReplace = j;

                                    break;
                                }
                            }

                            if (canReplace != size_t(-1)){
                                string varToReplace = globalNames[canReplace];

                                // can replace var[canReplace]
                                stateVecIdxs.erase(canReplace);
                                stateVecIdxs.insert(diffIdx);

                                // convert state var
                                auto left = make_shared<StringGraph>(dynamicVars[diffIdx].get(), globalNames[diffIdx]);
                                stateToDynVars[diffIdx]->getVariable(varToReplace, left);
                                // swap state vars
                                stateToDynVars[canReplace] = stateToDynVars[diffIdx];
                                stateToDynVars[diffIdx] = left;

                                // replace in equations
                                for(auto& eq:algSystem){
                                    eq->replaceVariable(varToReplace, stateToDynVars[canReplace]);
                                }
                                for(auto& eq:stateToDynVars){
                                    eq->replaceVariable(varToReplace, stateToDynVars[canReplace]);
                                }

                                // add to precessed list
                                diffInitValues.insert(diffIdx);

                                qDebug() << "Replaced " << varToReplace << " with " << stateToDynVars[canReplace]->toString();

                                break;
                            }
                        }

                        if(canReplace == size_t(-1)){
                            // can't restore this diff; therefore it's a dependent diff
                            // remove from diff rank
                            diffPresent.erase(diffIdx);
                            tmpVec.erase(tmpVec.begin()+k);

                            qDebug() << "gave up(";
                        }
                    }
                }
            }
        }
        qDebug() << "Init priority cycle: done!";

        // fill stateWsVector
        stateSize = stateVecIdxs.size(); // final stateSize
        stateVectorNames.resize(stateSize);
        size_t i = 0;
        for (auto stIdx:stateVecIdxs){
            stateVectorNames[i] = globalNames[stIdx];

            i++;
        }

        // replace differential vars
        // collect diff vars that need to be replaced
        
        // replace
        // TODO do you even need this? After
        for (auto& i:diffPresent){
            // check if needs to be replaced
            bool replaceDiff = true;
            for (auto& v:stateVecIdxs){
                if (v == i){
                    // diff is one of reference vars; no replacements needed
                    replaceDiff = false;
                    //stateToDynDiffs[i] = nullptr;
                    diffRank++;
                    break;
                }
            }

            if (replaceDiff){
                // check if depends on another differential
                auto vars = stateToDynVars[i]->getVariables();
                for (auto& v:vars){
                    if (v->getWsLink()->type() == WorkSpace::WsTypes::Differential){
                        throw runtime_error(dynDiffVars[i]->getName()+" depends on "+v->getName());
                    }
                }

                // replace Diff
                auto newDiff = stateToDynVars[i]->getFullTimeDiffer(globalNames,
                                                                     //dynamicVars,
                                                                     dynDiffVars);
                // construct global name of i'th diff
                auto namePath = XmlParser::split(globalNames[i], ELEM_PATH_DELIM);
                namePath.back() = dynDiffVars[i]->getName();
                string oldName = XmlParser::join(namePath, ELEM_PATH_DELIM);
                for(auto& eq:algSystem){
                    eq->replaceVariable(oldName, newDiff);
                }
                // replace also in state vector
                for(auto& eq:stateToDynVars){
                    eq->replaceVariable(oldName, newDiff);
                }

                #ifdef DEBUG
                logger->printMsg("Replaced "+oldName+" with "+newDiff->toString()+"\n");
                #endif

                stateToDynDiffs[dynDiffVars[i].get()] = newDiff; // Diff no longer exists

                // remove from present diffs (?)
                diffPresent.erase(i);
            }
        }

        // calculate ODE rank
        // note: is there more elegant solution?
        // diffPresent.clear();
        // for(auto& eq:algSystem){
        //     for(auto& v:eq->getVariables()){
        //         if (v->getWsLink()->type() == WorkSpace::Differential){
        //             // get var's index
        //             for (auto i=dynDiffVars.size(); i-->0;){
        //                 if (dynDiffVars[i].get() == v->getWsLink()){
        //                     diffPresent.insert(i);
        //                     break;
        //                 }
        //             }
        //         }
        //     }
        // }
        diffRank = diffPresent.size();

        return diffPresent;
    }

    void DAE::initJacobian(int jacobType){
        Jacob.clear();
        int i=0;
        for(auto& func:algSystem){
            Jacob.push_back(vector<shared_ptr<StringGraph>>());
            for(auto& var:stateVectorNames){
                Jacob[i].push_back(func->getDiffer(var));
            }
            i++;
        }

        // logging
        string varsLayout="[ ";
        for(auto& var:stateVectorNames) {
            varsLayout += var + " ";
        }
        varsLayout+="]";
        logger->printMsg("\nJacobian: "+varsLayout+" out of ???\n");
        for(auto& row:Jacob){
            for(auto& item:row){
                logger->printMsg(item->toString() + "     ");
            }
            logger->printMsg("\n");
        }


        if(jacobType==2) //jacob only
            return;
        if(!Jacob.empty()){
            invJacob = MatrixEqu::invMatr_s(getJacob());
            // logging
            logger->printMsg("\ninv Jacobian:\n");
            for(auto& row:Jacob){
                for(auto& item:row){
                    logger->printMsg(item->toString()+"     ");
                }
                logger->printMsg("\n");
            }
            if(jacobType==1) //inv jacob only
                return;
            newtonFunc=MatrixEqu::mulMatxToRow_s(invJacob, algSystem);
            // logging
            logger->printMsg("\nNewton's method func:\n");
            for(auto& row:newtonFunc){
                logger->printMsg(row->toString()+"\n");
            }
        }
    }

    void DAE::layout(){
        // File file=new File("C:\\NetBeansLogs\\XesLog.txt");
        // try (BufferedWriter bw = new BufferedWriter(new FileWriter(file,true))){
        //     for(int i=0;i<logtime.size();i++){
        //         bw.write(logtime.get(i).toString()+" ");
        //         for(vector<double> val:logdata){
        //             bw.write(val.get(i).toString()+" ");
        //         }
        //         bw.newLine();
        //     }
        // } catch (IOException e) { System.err.println(e.getMessage()); }
    }

    /**
        * @return the Jacob
        */
    vector<vector<shared_ptr<StringGraph>>> DAE::getJacob() {
        return Jacob;
    }

    /**
        * @return the invJacob
        */
    vector<vector<shared_ptr<StringGraph>>> DAE::getInvJacob() {
        return invJacob;
    }

    /**
        * @return the newtonFunc
        */
    vector<shared_ptr<StringGraph>> DAE::getNewtonFunc() {
        return newtonFunc;
    }
    
    void DAE::updateElemWorkspaces(){
        for (auto i=dynamicVars.size(); i-->0;){
            dynamicVars[i]->setValue(stateToDynVars[i]->evaluate());
        }
        for (auto& [dv, sg]:stateToDynDiffs){
            dv->setValue(sg->evaluate());
        }
    }
}
