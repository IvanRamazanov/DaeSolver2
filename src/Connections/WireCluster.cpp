/*
 * The MIT License
 *
 * Copyright 2018 Ivan.
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

#include "Wire.h"

using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;
using Pin = ElementBase::Pin;

namespace Connections{
    WireCluster::WireCluster(ElementBase::Subsystem* system, Domain *domain): domain(domain), system(system){
        // markers
        if (domain->directional){
            // first one is always an outbound line
            lineMarkers.push_back(make_unique<LineMarker>(this, Direction::Output));
            lineMarkers.push_back(make_unique<LineMarker>(this, Direction::Input));
        }else{
            lineMarkers.push_back(make_unique<LineMarker>(this));
            lineMarkers.push_back(make_unique<LineMarker>(this));
        }

        // arrange
        bindCrosses();
    }

    WireCluster::WireCluster(ElementBase::Subsystem *system, XmlParser::XmlElement *xml_data): system(system){
        if (xml_data->attributes.count(ATTR_DOMAIN))
            domain = Domains::Domain::getDomain(xml_data->attributes[ATTR_DOMAIN]);
        
        if (domain == nullptr)
            // set default domain
            domain = Domains::ELECTRIC.get();
        
        for (auto& lm:xml_data->findAll(LM_TAG)){
            lineMarkers.push_back(make_unique<LineMarker>(this, lm));
        }

        if (domain->directional){
            // verify that cluster has exactly one source marker
            bool srcFound = false;
            for (auto& lm:lineMarkers){
                if (lm->direction == Direction::Output){
                    if (!srcFound){
                        srcFound = true;
                    }else{
                        // already has source
                        lm->unPlug();
                        lm->direction = Direction::Input; 
                        // TODO: at the moment, marker is symmetrical, but ideally ref point should be also altered
                    }
                }
            }
            if (!srcFound){
                // Note: Subsystem verifies that Wire is not empty
                lineMarkers[0]->unPlug();
                lineMarkers[0]->direction = Direction::Output;
            }
        }

        // Cross2Cross
        for(auto& cc:xml_data->findAll(CL_TAG)){
            ContContList.push_back(make_unique<ConnectionLine>(this));
            ConnectionLine* newLine = ContContList.back().get();
            // restore position
            auto posInfo = cc->find(LINE_POS_TAG);
            if (posInfo && !posInfo->text.empty()){
                auto coords = XmlParser::split(posInfo->text, ";");
                if (coords.size() == 4){
                    QPointF st(stod(coords[0]), stod(coords[1])),
                            en(stod(coords[2]), stod(coords[3]));
                    newLine->getStartMarker()->moveCenterTo(st);
                    newLine->getEndMarker<Cross*>()->moveCenterTo(en);
                }
            }
        }

        auto node = xml_data->find("DotList");
        if (node){
            for (auto& entry:node->findAll("dotCluster")){
                vector<Cross*> line;
                if (entry->attributes.contains("lm")){
                    // fill dotList with Cross pointers based on indexes in lm attr
                    auto lmIdxs = XmlParser::parseRow(entry->attributes["lm"]);
                    for (auto& i:lmIdxs){
                        if (i>=0 && i<lineMarkers.size()){
                            line.push_back(lineMarkers[i]->getAnchor());
                        }
                    }
                }
                if (!ContContList.empty() && entry->attributes.contains("cl")){
                    auto clIdxs = XmlParser::parseRow(entry->attributes["cl"]);
                    for (auto& i:clIdxs){
                        size_t c2cIdx = (size_t)i / 2;
                        bool startCross = (size_t)i % 2;
                        if (c2cIdx < ContContList.size()){
                            if (startCross){
                                line.push_back(ContContList[c2cIdx]->getStartMarker());
                            }else{
                                line.push_back(ContContList[c2cIdx]->getEndMarker<Cross*>());
                            }
                        }
                    }
                }
                if (!line.empty())
                    dotList.push_back(line);
            }
        }else{
            // TODO restore missing dotList?
        }

        bindCrosses();
    }

    /**
     * Sets up first wire connection (for basic wire).
     * @returns unconnected marker
     */
    LineMarker* WireCluster::initConnection(Pin* pin){
        if (lineMarkers.size() != 2){
            throw runtime_error("Func initConnection supports only basic wires.");
        }

        if(domain->directional){
            // directional; find matching marker
            if (pin->isCompatible(lineMarkers[0].get())){
                lineMarkers[0]->plugIn(pin);
                return lineMarkers[1].get();
            }else if (pin->isCompatible(lineMarkers[1].get())){
                lineMarkers[1]->plugIn(pin);
                return lineMarkers[0].get();
            }else{
                throw invalid_argument("Pin is incompatible with all markers.");
            }
        }else{
            if (!pin->isCompatible(lineMarkers[0].get())){
                throw invalid_argument("Pin's domain is incompatible with wire.");
            }
            // Unidirectional; just connect the first one
            lineMarkers[0]->plugIn(pin);
            return lineMarkers[1].get();
        }
    }

    Domain* WireCluster::getDomain(){
        return domain;
    }
    
    LineMarker* WireCluster::activeWireConnect = nullptr;
    LineMarker* WireCluster::getActiveMarker(){
        return activeWireConnect;
    }

    void WireCluster::setActiveMarker(LineMarker* value){
        if (value != nullptr && activeWireConnect != nullptr){
            throw runtime_error("ActiveWireMarker is already set!");
        }

        activeWireConnect = value;
    }

    pair<size_t, size_t> WireCluster::findDot(Cross* element) {
        return MathPack::indexOf_mat(dotList, element);
    }

    void WireCluster::bindCrosses(){
        if (lineMarkers.size() < 2){
            // only one line (shouldn't be possible)
            return;
        }else if (lineMarkers.size() == 2){
            lineMarkers[0]->unbind();
            lineMarkers[1]->unbind();
            // basic line; bind to markers
            lineMarkers[0]->getLine()->getStartMarker()->unbind();
            lineMarkers[1]->getLine()->getStartMarker()->unbind();

            lineMarkers[0]->getLine()->getStartMarker()->bindToMarker(lineMarkers[0].get());
            lineMarkers[1]->getLine()->getStartMarker()->bindToMarker(lineMarkers[0].get());
        }else{
            // general case; bind to first cross
            for(auto& line:dotList){
                Cross* master = line.at(0);
                master->unbind();
                for(size_t i = 1; i<line.size(); i++){
                    // bind all crosses to a master cross
                    line[i]->unbind();
                    line[i]->bindToCross(master);
                }
            }
        }
    }

    XmlParser::XmlElement* WireCluster::to_xml(){
        XmlParser::XmlElement* bw = new XmlParser::XmlElement(WIRE_TAG);
        bw->attributes[ATTR_DOMAIN] = domain->typeName;

        size_t i=0;
        for(auto& lm:lineMarkers){
            bw->append(lm->to_xml(i));
            i++;
        }
        // write wire marker owner and index, if exists

        i=0;
        for(auto& lm:ContContList){
            bw->append(lm->to_xml());
            i++;
        }

        if (!dotList.empty()){
            auto node = new XmlParser::XmlElement("DotList");
            for (auto& line:dotList){
                auto cluster = new XmlParser::XmlElement("dotCluster");
                vector<size_t> lm, cl;
                // convert pointers to indexes in lineMarkers (or ContContList)
                for (auto& dot:line){
                    size_t idx = -1;
                    for (size_t i=0; i<lineMarkers.size(); i++){
                        if (lineMarkers[i]->getAnchor() == dot){
                            idx = i;
                            break;
                        }
                    }
                    if (idx != size_t(-1)){
                        lm.push_back(idx);
                        continue;
                    }
                    // not found in markers, has to be C2C Cross
                    for (size_t i=0; i<ContContList.size(); i++){
                        if (ContContList[i]->getStartMarker() == dot){
                            idx = i*2;
                            break;
                        }
                        if (ContContList[i]->getEndMarker<Cross*>() == dot){
                            idx = i*2+1;
                            break;
                        }
                    }
                    if (idx != size_t(-1)){
                        cl.push_back(idx);
                    }
                }
                if (!lm.empty())
                    cluster->attributes["lm"] = XmlParser::vec_to_s(lm);
                if (!cl.empty())
                    cluster->attributes["cl"] = XmlParser::vec_to_s(cl);
                node->append(cluster);
            }
            
            bw->append(node);
        }

        return bw;
    }

    ElementBase::Subsystem* WireCluster::getSystem(){
        return system;
    }

    void WireCluster::setSystem(ElementBase::Subsystem* sys){
        system = sys;
    }

    void WireCluster::consumeWire(ConnectionLine* eventSource){
        Q_UNUSED(eventSource);
        // MouseDragEvent nEvent=mde.copyFor(mde.getGestureSource(),mde.getTarget(), MouseDragEvent.MOUSE_DRAG_RELEASED);

        // double  x=mde.getX(),
        //         y=mde.getY();
        // auto consumedWire = activeWireConnect->getWire();

        // wireConsumptionBackup->index = MathPack::indexOf(consumedWire->lineMarkers, activeWireConnect);
        // wireConsumptionBackup->class_name = getClass().getName();
        // wireConsumptionBackup->consumed_wire_data = consumedWire->to_xml();
        // wireConsumptionBackup->master_wire_data = this->to_xml();

        // switch(consumedWire->getRank()){
        //     case 1:
        //         activeWireConnect->setWire(this);

        //         // add to this wire
        //         lineMarkers.push_back(activeWireConnect);

        //         consumedWire->lineMarkers.remove(0);
        //         consumedWire->destroy();  // remove empty wire
        //         //flip
        //         activeWireConnect->getLine()->getStartMarker()->unbind();
        //         activeWireConnect->setStartPoint(x,y);
        //         activeWireConnect->bindElemContact(activeWireConnect->getConnectedPin());

        //         switch(size()) {
        //             case 1+1:
        //             {
        //                 LineMarker wm = addLineMarker(this, x, y);
        //                 // adjustment
        //                 vector<Cross*> row;
        //                 row.push_back(activeWireConnect->getLine()->getStartMarker());
        //                 row.push_back(eventSource.getLine()->getStartMarker());
        //                 row.push_back(wm.getLine()->getStartMarker());
        //                 dotList.push_back(row);
        //                 bindCrosses();

        //                 // move before rebind
        //                 wm.getBindX()->setValue(eventSource.getBindX()->getValue());
        //                 wm.getBindY()->setValue(eventSource.getBindY()->getValue());
        //                 eventSource.bindElemContact(eventSource.getConnectedPin());
        //                 break;
        //             }
        //             case 2+1:
        //             {
        //                 // adjustment
        //                 vector<Cross*> row;
        //                 row.push_back(activeWireConnect->getLine()->getStartMarker());
        //                 row.push_back(lineMarkers.at(0)->getLine()->getStartMarker());
        //                 row.push_back(lineMarkers.at(1)->getLine()->getStartMarker());
        //                 dotList.push_back(row);
        //                 bindCrosses();
        //                 showAll();
        //                 break;
        //             }
        //             default:
        //             {
        //                 addContToCont(eventSource.getItsLine()->getStartMarker(), activeWireConnect->getItsLine()->getStartMarker());
        //                 break;
        //             }
        //         }
        //         eventCross = activeWireConnect->getItsLine()->getStartMarker();
        //         activeWireConnect = nullptr;
        //         break;
        //     case 2:
        //         // TODO This case is present, when fully unplugged wire connects. Also MathWire case.
        //         if(consumedWire->lineMarkers.at(0)->getConnectedPin() == nullptr &&
        //                 consumedWire->lineMarkers.at(1)->getConnectedPin() == nullptr) {
        //             // fully disconnected case
        //         }else{
        //             // MathWire case?
        //             if(consumedWire->lineMarkers.at(1)->getConnectedPin() != nullptr){ //make sure that is mathwire case

        //             }
        //         }
        //         eventCross.reset();
        //         break;
        //     default:
        //         int sx=activeWireConnect->getStartX()->getValue(),
        //             sy=activeWireConnect->getStartY()->getValue();
        //         int rank = getRank();
        //         // merge lists
        //         auto p = MathPack::MatrixEqu::findFirst(consumedWire->dotList, activeWireConnect->getLine()->getStartMarker());
        //         p = p.add(dotList.size(), 0);
        //         dotList.addAll(consumedWire->dotList);
        //         consumedWire->lineMarkers.remove(activeWireConnect);
        //         lineMarkers.addAll(consumedWire->lineMarkers);
        //         for(auto& lm : consumedWire->lineMarkers){
        //             lm->setWire(this);
        //         }
        //         consumedWire->lineMarkers.clear();
        //         getContContList().addAll(consumedWire->getContContList());
        //         for(auto& lm:consumedWire->getContContList()){
        //             lm->setWire(this);
        //         }
        //         consumedWire->getContContList().clear();
        //         consumedWire->destroy();

        //         // replace with crosToCros
        //         auto replacementLine = addContToCont(sx, sy, x, y);
        //         dotList.at(p.first).at(p.second) = replacementLine->getStartMarker();
        //         activeWireConnect->destroy();
        //         activeWireConnect = nullptr;
        //         switch(rank){
        //             case 1:
        //                 //TODO u know what to do
        //                 break;
        //             case 2:
        //             {
        //                 vector<Cross*> row;
        //                 row.push_back(lineMarkers.at(0)->getLine()->getStartMarker());
        //                 row.push_back(replacementLine->getEndCrossMarker());
        //                 row.push_back(lineMarkers.at(1)->getLine()->getStartMarker());
        //                 dotList.push_back(row);
        //                 bindCrosses();
        //                 showAll();
        //                 break;
        //             }
        //             default:
        //                 addContToCont(eventSource.getLine()->getStartMarker(), replacementLine->getEndCrossMarker());
        //         }
        //         eventCross=replacementLine->getEndCrossMarker();
        // }
        // eventCross.addEventFilter(MouseDragEvent.MOUSE_DRAG_EXITED,mouseExit);
        // eventCross.addEventFilter(MouseDragEvent.MOUSE_DRAG_RELEASED,mouseReleased);

        // ((Node)mde.getGestureSource()).fireEvent(nEvent);
    }

    LineMarker* WireCluster::getMarker(size_t idx){
        return lineMarkers.at(idx).get();
    }

    const vector<unique_ptr<LineMarker>>& WireCluster::getMarkers(){
        return lineMarkers;
    }

    QList<QGraphicsItem*> WireCluster::getView(){
        QList<QGraphicsItem*> out;
        for(auto& lm:lineMarkers){
            out.append(lm->getView());
        }
        for(auto& ctcl:ContContList){
            out.append(ctcl->getView());
        }

        return out;
    }

    LineMarker* WireCluster::findMarker(Direction dir){
        for (auto& lm:lineMarkers){
            if (lm->direction == dir)
                return lm.get();
        }
        throw runtime_error("Marker with direction: " + Domains::dir2s(dir) + " not found!");
    }

    size_t WireCluster::size(){
        return(lineMarkers.size());
    }

    LineMarker* WireCluster::addLineMarker(ConnectionLine* line, QPointF const& pos){
        Direction dir = domain->directional ? Direction::Input : Direction::Uni;
        size_t original_size = size();
        lineMarkers.push_back(make_unique<LineMarker>(this, dir));
        
        unique_ptr<LineMarker> &newMarker = *(lineMarkers.end()-1);

        // TODO get (closest) point on the line
        newMarker->getAnchor()->moveCenterTo(pos);

        // adjust crosses
        if (line->isCrossToCross()){
            // insert into the line

            // make new cross to cross line
            ContContList.push_back(make_unique<ConnectionLine>(this));
            ConnectionLine* newLine = ContContList.back().get();

            // find old bindings
            auto oldLine_start = findDot(line->getStartMarker());
            
            // rebind to new marker's line
            // old move start Cross to new line
            dotList[oldLine_start.first].erase(dotList[oldLine_start.first].begin() + oldLine_start.second);
            dotList[oldLine_start.first].push_back(newLine->getStartMarker());
            // group the rest at new point
            dotList.push_back(vector<Cross*>{newMarker->getAnchor(),
                                            newLine->getEndMarker<Cross*>(),
                                            line->getStartMarker()
                                            }
                            );

             // add graphics items to subsystem scene
            auto itemList = newLine->getView();
            for (auto& i: itemList)
                system->_getScene()->addItem(i);               
        }else if (original_size == 2){
            // insert between markers (bind all 3 together)
            dotList.push_back(vector<Cross*>{newMarker->getAnchor(),
                                            lineMarkers[0]->getAnchor(),
                                            lineMarkers[1]->getAnchor()
                                            }
                            );
        }else{
            // cut the line; replace it's end with CrossToCross line

            // make new cross to cross line
            ContContList.push_back(make_unique<ConnectionLine>(this));
            ConnectionLine* newLine = ContContList.back().get();

            // find old bindings
            auto oldStart = findDot(line->getStartMarker());

            // rebind to new marker's line
            // old move start Cross to new line
            dotList[oldStart.first].erase(dotList[oldStart.first].begin() + oldStart.second);
            dotList[oldStart.first].push_back(newLine->getStartMarker());
            // group the rest at new point
            dotList.push_back(vector<Cross*>{newMarker->getAnchor(),
                                            newLine->getEndMarker<Cross*>(),
                                            line->getStartMarker()
                                            }
                            );

            // add graphics items to subsystem scene
            auto itemList = newLine->getView();
            for (auto& i: itemList)
                system->_getScene()->addItem(i);
        }
        bindCrosses();

        // adjust position
        newMarker->moveCenterTo(pos);

        // add graphics items to subsystem scene
        auto itemList = newMarker->getView();
        for (auto& i: itemList)
            system->_getScene()->addItem(i);

        return newMarker.get();
    }

    LineMarker* WireCluster::addLineMarker(ElementBase::Pin *pin){
        auto lastLine = lineMarkers.back()->getLine();
        // take the middle point of line coords
        auto stPos = lastLine->getStartMarker()->scenePos(),
            endPos = lastLine->isCrossToCross() ? lastLine->getEndMarker<Cross*>()->scenePos() : \
                                                lastLine->getEndMarker<LineMarker*>()->scenePos();
        QPointF rootPos = (endPos+stPos)/2;
        auto ret = addLineMarker(lastLine, rootPos);
        ret->plugIn(pin);
        
        return ret;
    }

    void WireCluster::remove(ConnectionLine* line){
        if (line->isCrossToCross()){
            // cross to cross line case. Remove everything(?)
            system->removeWire(this);
        }else{
            // marker line
            LineMarker *endMarker = line->getEndMarker<LineMarker*>();
            
            if (size() == 2){
                // remove entire wire
                system->removeWire(this);
                return;
            }
            
            if(size() == 3){
                // no c2c exist.
                // simply remove LM's dot (should be only one in the list)
                dotList.clear();
            }else{
                // has C2C line.
                auto dot = findDot(line->getStartMarker());
                ConnectionLine *c2c = nullptr; // c2c line to remove      
                Cross *otherCross = nullptr,
                        *preserveCross = nullptr; // cross of the line that will be "spared"
                // classify crosses in the dot
                for (size_t i=0; i<dotList[dot.first].size(); i++){
                    // skip main line (marker)
                    if (i != dot.second){
                        if(c2c==nullptr && dotList[dot.first][i]->owner->isCrossToCross()){
                            // replace
                            c2c = dotList[dot.first][i]->owner;
                            otherCross = dotList[dot.first][i] == c2c->getStartMarker() ? \
                                                c2c->getEndMarker<Cross*>() : c2c->getStartMarker();
                        }else{
                            preserveCross = dotList[dot.first][i]->owner->getStartMarker();
                        }
                    }
                }
                // rebind crosses
                auto otherDot = findDot(otherCross);
                dotList[otherDot.first][otherDot.second] = preserveCross;
                // remove c2c line
                MathPack::remove(ContContList, c2c);
                // remove dot (doesn't bind anything anymore)
                dotList.erase(dotList.begin() + dot.first);
            }

            // remove Marker
            MathPack::remove(lineMarkers, endMarker);

            bindCrosses();
        }
    }

    double* WireCluster::getDataRef(){
        return data;
    }
    size_t WireCluster::getDataSize(){
        return dataSize;
    }  
    /**
     * Note: Wire doesn't own the data. Data is located in Workspace
     */
    void WireCluster::initDataRef(double *dataRef, size_t size){
        data = dataRef;
        dataSize = size;

        // pass to inputs, if connected
        for (auto& lm:lineMarkers){
            if (lm->isPlugged() && lm->getConnectedPin()->getDirection() == Domains::Input){
                lm->getConnectedPin()->data = data;
                lm->getConnectedPin()->dataSize = dataSize;
            }
        }
    }
}
