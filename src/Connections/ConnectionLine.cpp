/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Wire.h"

using namespace std;
using namespace MathPack;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;

constexpr double ZERO_LINE_EPS = 1;
constexpr auto CL_POSITIONS_TAG = "Constraints";
bool hasZeroLength(QLineF const& line){
    return line.dx() < ZERO_LINE_EPS && line.dy() < ZERO_LINE_EPS;
}

namespace Connections{
    /**
     *
     * @author Ivan
     */
    
    /**
     * class ConnectionLine
     */

        /**
         *
         * @param sX - start X
         * @param sY - start Y
         * @param eX - end X
         * @param eY - end Y
         * @param horizontal - horizontal or vertical line
         */
        Line::Line(ConnectionLine* owner, double sX, double sY, double eX, double eY, bool horizontal): horizontal(horizontal), 
                                                                                                        owner(owner)
        {
            // draw line inside widget
            setLine(sX, sY, eX, eY);

            // line styling
            setActive(owner->isActive, owner->color);
            this->setCursor(Qt::PointingHandCursor);
            
            //setFocusPolicy(Qt::ClickFocus);
            setFlags(QGraphicsItem::ItemIsFocusable);
        }
        
        Line::~Line(){
            // remove itself from scene
            owner->getWire()->getSystem()->_getScene()->removeItem(this);
        }

        void Line::dragDetected(QGraphicsSceneMouseEvent *event){
            auto m = owner->getWire()->addLineMarker(owner, event->pos());

            // transfer events to the new marker
            m->inheritMouseMove();
        }

        // events
        void Line::mousePressEvent(QGraphicsSceneMouseEvent *event){
            dragStartPosition = event->pos();
            mouseWasPressed = true;
            event->setAccepted(true);
        }

        void Line::mouseMoveEvent(QGraphicsSceneMouseEvent *event) { 
            if(!mouseWasPressed){
                return;
            }

            if (event->buttons() == Qt::RightButton){
                if ((event->pos() - dragStartPosition).manhattanLength() < 1)
                    return;

                mouseWasPressed = false;
                dragDetected(event);
            }else if(event->buttons() == Qt::LeftButton){
                // filter unnecessary draw calls
                bool doDraw = false;
                auto newPos = event->scenePos();
                if (horizontal){
                    doDraw = abs(line().y1()-newPos.y()) > ZERO_LINE_EPS;
                }else{
                    doDraw = abs(line().x1()-newPos.x()) > ZERO_LINE_EPS;
                }
                if (doDraw){
                    owner->shiftLine(this, newPos);
                }
            }
        }
                    
        void Line::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
            Q_UNUSED(event);
            mouseWasPressed = false;
        }

        void Line::dragMoveEvent(QGraphicsSceneDragDropEvent *event){
            // ??? Don't need this?
            Q_UNUSED(event);
        }

        void Line::dragEnterEvent(QGraphicsSceneDragDropEvent *event){
            Q_UNUSED(event);
            if(WireCluster::getActiveMarker() != nullptr && WireCluster::getActiveMarker()->getDomain() == owner->getWire()->getDomain()){
                // same domains
                // TODO check direction?
                if(WireCluster::getActiveMarker()->getWire() != owner->getWire()){
                    owner->getWire()->consumeWire(owner);
                }
            }
        }

        void Line::dragLeaveEvent(QGraphicsSceneDragDropEvent *event){
            Q_UNUSED(event);
        }

        // void Line::focusInEvent(QFocusEvent* event){
        //     Q_UNUSED(event);
        //     //this.setEffect(new DropShadow(BlurType.GAUSSIAN, Color.AQUA, 2, 1, 0, 0));
        // }

        // void Line::focusOutEvent(QFocusEvent* event){
        //     Q_UNUSED(event);
        //     //this.setEffect(null);
        // }

        void Line::keyReleaseEvent(QKeyEvent *event){
            if(event->key()==Qt::Key_Delete){
                owner->getWire()->remove(owner);
            }
        }

        

        /**
         * @return the horizontal
         */
        bool Line::isHorizontal() {
            return horizontal;
        }

        void Line::setActive(bool isActive, QColor const& activeColor){
            if (isActive){
                setPen(QPen(QBrush(activeColor), 2,  Qt::SolidLine));
            }else{
                setPen(QPen(QBrush(Qt::red), 2,  Qt::DashLine));
            }
        }
        /* END of ConnectionLine::Line*/


    ConnectionLine::ConnectionLine(LineMarker* anchor, XmlParser::XmlElement *data){
        domain = anchor->getDomain();
        owner = anchor->getWire();
        marker = anchor;

        init(data);

        // bind to marker
        anchor->getBindPoint()->bind(endPoint);
    }

    ConnectionLine::ConnectionLine(WireCluster *owner): owner(owner){
        domain = owner->getDomain();

        //Cross *endCross = new Cross(this);
        marker = make_unique<Cross>(this);

        init();

        // bind to marker
        std::get<unique_ptr<Cross>>(marker)->centerPoint->bind(endPoint);
    }

    ConnectionLine::~ConnectionLine(){
        startPoint->unbind();
        endPoint->unbind();
    }

    /**
     * @param data - xml data from save file
     */
    void ConnectionLine::init(XmlParser::XmlElement *data){
        color = domain->wireColour;

        if (data==nullptr){
            // default lines
            startPoint = make_shared<Property<QPointF>>(QPointF());
            endPoint = make_shared<Property<QPointF>>(QPointF());

            // lines
            lines.push_back(make_unique<Line>(this, 0,0 , 0,0 , true));
            lines.push_back(make_unique<Line>(this, 0,0 , 0,0 , false));
            lines.push_back(make_unique<Line>(this, 0,0 , 0,0 , true));
        }else{
            // restore from save file
            size_t numOfLines = 3;
            if (data->attributes.count("size"))
                numOfLines = stoi(data->attributes["size"]);
            bool horiz = true;
            if (data->attributes.count("horizontal"))
                numOfLines = XmlParser::stob(data->attributes["horizontal"]);

            auto endPoints = data->find(LINE_POS_TAG);
            if (endPoints){
                auto coords = XmlParser::split(endPoints->text, ";");
                while (coords.size() < 4){
                    coords.push_back("0");
                }
                startPoint = make_shared<Property<QPointF>>(QPointF(stod(coords[0]), stod(coords[1])));
                endPoint = make_shared<Property<QPointF>>(QPointF(stod(coords[2]), stod(coords[3])));
            }else{
                startPoint = make_shared<Property<QPointF>>(QPointF());
                endPoint = make_shared<Property<QPointF>>(QPointF());
            }

            auto posns = data->find(CL_POSITIONS_TAG);
            if (posns){
                auto tmp = XmlParser::parseRow(posns->text);
                for (auto& p:tmp)
                    linePositions.append(p);

                if (!linePositions.empty()){
                    // override lines length. Trust constraints
                    numOfLines = linePositions.size() + 2;
                }else{
                    numOfLines = max(numOfLines, size_t(3));
                }

                // construct lines
                // start
                if (linePositions.empty()){
                    // no constraints
                    if (numOfLines == 2){
                        auto p1 = startPoint->getValue(),
                            p2 = endPoint->getValue();
                        lines.push_back(make_unique<Line>(this, 
                            p1.x(),
                            p1.y(),
                            (p1.x()+p2.x())/2,
                            p1.y(),
                            horiz));
                        lines.push_back(make_unique<Line>(this,
                            (p1.x()+p2.x())/2,
                            p2.y(),
                            p2.x(),
                            p2.y(),
                            !horiz));
                    }else{
                        // default lines
                        auto p1 = startPoint->getValue(),
                            p2 = endPoint->getValue();
                        lines.push_back(make_unique<Line>(this, 
                            p1.x(),
                            p1.y(),
                            (p1.x()+p2.x())/2,
                            p1.y(),
                            horiz));
                        lines.push_back(make_unique<Line>(this,
                            (p1.x()+p2.x())/2,
                            p1.y(),
                            (p1.x()+p2.x())/2,
                            p2.y(),
                            !horiz));
                        lines.push_back(make_unique<Line>(this,
                            (p1.x()+p2.x())/2,
                            p2.y(),
                            p2.x(),
                            p2.y(),
                            horiz));
                    }
                }else{
                    // apply constraints
                    lines.push_back(make_unique<Line>(this,
                                                    startPoint->getValue().x(),
                                                    startPoint->getValue().y(),
                                                    horiz ? linePositions[0] : startPoint->getValue().x(),
                                                    horiz ? startPoint->getValue().y() : linePositions[0],
                                                    horiz));
                    auto preLine = lines.back().get();
                    for (int i=0; i<linePositions.size(); i++){
                        horiz = !horiz;
                        // end point
                        double eX, eY;
                        if (i+1 <linePositions.size()){
                            // still in the middle of array
                            eX = horiz ? linePositions[i+1] : preLine->line().x2();
                            eY = horiz ? preLine->line().y2() : linePositions[i+1];
                        }else{
                            eX = horiz ? endPoint->getValue().x() : preLine->line().x2();
                            eY = horiz ? preLine->line().y2() : endPoint->getValue().y();
                        }
                        lines.push_back(make_unique<Line>(this,
                                                        preLine->line().x2(),
                                                        preLine->line().y2(),
                                                        eX,
                                                        eY,
                                                        horiz));
                        preLine = lines.back().get();
                    }
                    // last
                    horiz = !horiz;
                    lines.push_back(make_unique<Line>(this,
                                                horiz ? preLine->line().x2() : endPoint->getValue().x(),
                                                horiz ? endPoint->getValue().y() : preLine->line().y2(),
                                                endPoint->getValue().x(),
                                                endPoint->getValue().y(),
                                                horiz));
                }
            }else{
                // default lines
                auto p1 = startPoint->getValue(),
                    p2 = endPoint->getValue();
                lines.push_back(make_unique<Line>(this, 
                    p1.x(),
                    p1.y(),
                    (p1.x()+p2.x())/2,
                    p1.y(),
                    true));
                lines.push_back(make_unique<Line>(this,
                    (p1.x()+p2.x())/2,
                    p1.y(),
                    (p1.x()+p2.x())/2,
                    p2.y(),
                    false));
                lines.push_back(make_unique<Line>(this,
                    (p1.x()+p2.x())/2,
                    p2.y(),
                    p2.x(),
                    p2.y(),
                    true));
            }

            // TODO dot index!
        }

        // start marker (Cross)
        startMarker = make_unique<Cross>(this, startPoint->getValue());
        startMarker->centerPoint->bind(startPoint);

        // prop changes
        startPoint->setOnChange([this](QPointF oldV, QPointF newV){
            Q_UNUSED(oldV);
            moveEndTo(newV, true);
        });

        endPoint->setOnChange([this](QPointF oldV, QPointF newV){
            Q_UNUSED(oldV);
            moveEndTo(newV, false);
        });
    }

    XmlParser::XmlElement* ConnectionLine::to_xml(){
        XmlParser::XmlElement* ret = new XmlParser::XmlElement(CL_TAG);
        ret->attributes["size"] = to_string(lines.size());
        ret->attributes["horizontal"] = to_string(lines.at(0)->isHorizontal());
        
        XmlParser::XmlElement* node = new XmlParser::XmlElement(LINE_POS_TAG);
        auto endP = endPoint->getValue(),
            startP = startPoint->getValue();
        node->text = std::format("{};{};{};{}",
            startP.x(),
            startP.y(),
            endP.x(),
            endP.y());
        ret->append(node);

        if(linePositions.size()){
            // array of constraints
            node = new XmlParser::XmlElement(CL_POSITIONS_TAG);
            node->text = XmlParser::vec_to_s(linePositions);
            ret->append(node);
        }

        return ret;
    }

    void ConnectionLine::setVisible(bool val){
        for(auto& lin:lines){
            lin->setVisible(val);
        }
    }

    void ConnectionLine::toFront(){
        for(auto& lin:lines){
            lin->setZValue(0); // TODO
        }
    }

    void ConnectionLine::toBack(){
        bool flag = false;
        for(auto& lin:lines){
            if(flag) { 
                // leave start line on front
                //l->lower();
                lin->setZValue(0); // TODO
            }else{
                //l->raise();
                lin->setZValue(0); // TODO
                flag = true;
            }
        }
    }

    WireCluster* ConnectionLine::getWire(){
        return owner;
    }

    void ConnectionLine::moveEndTo(QPointF const& newPos, bool firstLine){
        size_t endIdx = lines.size()-1;
        if (firstLine){
            auto tmp = lines[0]->line(),
                tmp2 = lines[1]->line();
            tmp.setP1(newPos);
            if (endIdx == 2 && linePositions.empty()){
                // 3 lines and no constraints. Line in the middle is floating
                auto enL = lines[endIdx]->line();
                auto enP = enL.p2();
                if (lines[0]->isHorizontal()){
                    auto middleX = (enP.x()+newPos.x())/2;
                    // Note: since end Line is horiz, therefore start Line is also horiz
                    enL.setP1(QPointF(middleX, enP.y()));
                    tmp.setP2(QPointF(middleX, newPos.y()));
                }else{
                    auto middleY = (enP.y()+newPos.y())/2;
                    enL.setP1(QPointF(enP.x(), middleY));
                    tmp.setP2(QPointF(newPos.x(), middleY));
                }
                // move middle line
                tmp2.setP2(enL.p1());
                tmp2.setP1(tmp.p2());

                // update lines
                lines[0]->setLine(tmp);
                lines[1]->setLine(tmp2);
                lines[2]->setLine(enL);
            }else{
                // general case. Adjust only this line
                if (lines[0]->isHorizontal()){
                    tmp.setP2(QPointF(tmp2.x1(), newPos.y()));
                    tmp2.setP1(QPointF(tmp2.x1(), newPos.y()));
                }else{
                    tmp.setP2(QPointF(newPos.x(), tmp2.y1()));
                    tmp2.setP1(QPointF(newPos.x(), tmp2.y1()));
                }
                lines[0]->setLine(tmp);
                lines[1]->setLine(tmp2);
            }
        }else{
            QLineF  tmp = lines[endIdx]->line(),
                    tmp2 = lines[endIdx-1]->line();
            tmp.setP2(newPos);
            if (endIdx == 2 && linePositions.empty()){
                // 3 lines and no constraints. Line in the middle is floating
                auto stL = lines[0]->line();
                auto stP = stL.p1();
                if (lines[endIdx]->isHorizontal()){
                    auto middleX = (stP.x()+newPos.x())/2;
                    // Note: since end Line is horiz, therefore start Line is also horiz
                    stL.setP2(QPointF(middleX, stP.y()));
                    tmp.setP1(QPointF(middleX, newPos.y()));
                }else{
                    auto middleY = (stP.y()+newPos.y())/2;
                    stL.setP2(QPointF(stP.x(), middleY));
                    tmp.setP1(QPointF(newPos.x(), middleY));
                }
                // move middle line
                tmp2.setP1(stL.p2());
                tmp2.setP2(tmp.p1());

                // update lines
                lines[0]->setLine(stL);
                lines[1]->setLine(tmp2);
                lines[2]->setLine(tmp);
            }else{
                // general case. Adjust only this line
                if (lines[endIdx]->isHorizontal()){
                    tmp.setP1(QPointF(tmp2.x2(), newPos.y()));
                    tmp2.setP2(QPointF(tmp2.x2(), newPos.y()));
                }else{
                    tmp.setP1(QPointF(newPos.x(), tmp2.y2()));
                    tmp2.setP2(QPointF(newPos.x(), tmp2.y2()));
                }
                lines[endIdx]->setLine(tmp);
                lines[endIdx-1]->setLine(tmp2);
            }
        }
    }

    /**
     * Update or create position constaint for selected line.
     */
    void ConnectionLine::shiftLine(Line *line, QPointF const& newPos){
        if (linePositions.empty()){
            // create
            bool horiz = lines[0]->isHorizontal();
            for (size_t i=1; i<lines.size()-1; i++){
                horiz = !horiz;

                if (horiz){
                    linePositions.append(lines[i]->line().y1());
                }else{
                    linePositions.append(lines[i]->line().x1());
                }
            }
        }

        auto endIdx = lines.size() - 1;
        for (size_t i = endIdx+1; i-->0;){
            if (line == lines[i].get()){
                // found the line
                if (i == endIdx){
                    // last line; binded to endPoint
                    // create ne line, since last line can't be moved
                    if (line->isHorizontal()){
                        linePositions.append(newPos.y());
                        lines.push_back(make_unique<Line>(this, 0,0, line->line().x2(), line->line().y2(), false));
                    }else{
                        linePositions.append(newPos.x());
                        lines.push_back(make_unique<Line>(this, 0,0, line->line().x2(), line->line().y2(), true));
                    }
                }else if (i == 0){
                    // first line, binded to startPoint
                    if (line->isHorizontal()){
                        linePositions.push_front(newPos.y());
                        lines.push_front(make_unique<Line>(this, line->line().x1(), line->line().y1(), 0,0, false));
                    }else{
                        linePositions.push_front(newPos.x());
                        lines.push_front(make_unique<Line>(this, line->line().x1(), line->line().y1(), 0,0, true));
                    }
                    i++;
                }else{
                    // somewhere in the middle
                    if (line->isHorizontal()){
                        linePositions[i-1] = newPos.y();
                    }else{
                        linePositions[i-1] = newPos.x();
                    }
                }

                redrawLines(i);

                break;
            }
        }
    }

    void ConnectionLine::redrawLines(size_t lineIdx){
        if (linePositions.empty()){
            cerr << "redrawLines requires constraints!" << endl;
            return;
        }

        // Note: by design, if linePositions is not empty, we have at least 3 lines; 
        // and lineIdx won't be at the ends
        auto mainLine = lines[lineIdx]->line(),
            preLine = lines[lineIdx-1]->line(),
            aftLine = lines[lineIdx+1]->line();

        // update coordinates
        if (lines[lineIdx]->isHorizontal()){
            mainLine.setLine(
                preLine.x1(),
                linePositions[lineIdx-1],
                aftLine.x2(),
                linePositions[lineIdx-1]
            );
        }else{
            mainLine.setLine(
                linePositions[lineIdx-1],
                preLine.y1(),
                linePositions[lineIdx-1],
                aftLine.y2()
            );
        }
        // adjacent lines
        preLine.setP2(mainLine.p1());
        aftLine.setP1(mainLine.p2());

        if (lines.size() >= 3){
            // reduce near zero lines
            if (hasZeroLength(preLine)){
                bool hasPrePre = lineIdx-1 != 0; // has line at idx: -2 
                // adjust start position (to match exactly)
                if (hasPrePre){
                    // "consume" 2 lines
                    mainLine.setP1(lines[lineIdx-2]->line().p1());
                }else{
                    mainLine.setP1(preLine.p1());
                }
                
                // adjust end position
                if (lines[lineIdx]->isHorizontal()){
                    mainLine.setP2(QPointF(mainLine.x2(), mainLine.y1()));
                }else{
                    mainLine.setP2(QPointF(mainLine.x1(), mainLine.y2()));
                }
                // update line after
                aftLine.setP1(mainLine.p2());

                // update graphics
                lines[lineIdx]->setLine(mainLine);
                lines[lineIdx+1]->setLine(aftLine);
                
                // remove lines
                lines.erase(lines.begin()+lineIdx-1);
                if (hasPrePre){
                    // another preceeding line exists
                    lines.erase(lines.begin()+lineIdx-2);
                }
                // remove constraints
                if (hasPrePre){
                    // if it's 3rd line; then remove self constraint
                    size_t remIdx = lineIdx-2 == 0 ? 0 : lineIdx-3;
                    linePositions.removeAt(remIdx);
                    linePositions.removeAt(remIdx);
                }else{
                    // remove only self constraint
                    linePositions.removeAt(lineIdx-1);
                }

                return;
            }else
            if (hasZeroLength(aftLine)){
                bool hasAftAft = lineIdx+2 < lines.size(); // has line at idx: +2 
                // adjust end position (to match exactly)
                if (hasAftAft){
                    // "consume" 2 lines
                    mainLine.setP2(lines[lineIdx+2]->line().p2());
                }else{
                    mainLine.setP2(preLine.p2());
                }
                
                // adjust start position
                if (lines[lineIdx]->isHorizontal()){
                    mainLine.setP1(QPointF(mainLine.x1(), mainLine.y2()));
                }else{
                    mainLine.setP1(QPointF(mainLine.x2(), mainLine.y1()));
                }
                // update line before
                preLine.setP2(mainLine.p1());

                // update graphics
                lines[lineIdx]->setLine(mainLine);
                lines[lineIdx-1]->setLine(preLine);
                
                // remove lines
                lines.erase(lines.begin()+lineIdx+1);
                if (hasAftAft){
                    // another preceeding line exists
                    lines.erase(lines.begin()+lineIdx+2);
                }
                // remove constraints
                if (hasAftAft){
                    // if it's 3rd line; then remove self constraint
                    size_t remIdx = lineIdx+2 == lines.size()-1 ? lineIdx-1 : lineIdx;
                    linePositions.removeAt(remIdx);
                    linePositions.removeAt(remIdx);
                }else{
                    // remove only self constraint
                    linePositions.removeAt(lineIdx-1);
                }

                return;
            }
        }
        
        // no deductions; refresh graphics
        lines[lineIdx]->setLine(mainLine);
        lines[lineIdx-1]->setLine(preLine);
        lines[lineIdx+1]->setLine(aftLine);
    }

    void ConnectionLine::setActive(bool val){
        for(auto& line:lines){
            line->setActive(val, color);
        }
        isActive = val;
    }    

    bool ConnectionLine::isCrossToCross(){
        return holds_alternative<unique_ptr<Cross>>(marker);
    }

    template <> LineMarker* ConnectionLine::getEndMarker<LineMarker*>(){
        return get<LineMarker*>(marker);
    }

    template <> Cross* ConnectionLine::getEndMarker<Cross*>(){
        return get<unique_ptr<Cross>>(marker).get();
    }

    QList<QGraphicsItem*> ConnectionLine::getView(){
        QList<QGraphicsItem*> out;

        for(auto& line:lines){
            out.append(line.get());
        }
        out.append(startMarker.get());
        if(isCrossToCross())
            out.append(getEndMarker<Cross*>());

        return out;
    }

    /**
     * @return the crossMarker
     */
    Cross* ConnectionLine::getStartMarker() {
        return startMarker.get();
    }

}

