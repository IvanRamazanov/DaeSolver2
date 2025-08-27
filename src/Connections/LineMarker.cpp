/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "Wire.h"

using namespace std;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;

namespace Connections{
    /**
     *  class LineMarker
     * @author Ivan
     */

    LineMarker::LineMarker(){
        setCursor(Qt::PointingHandCursor);
        setAcceptHoverEvents(true);
        setZValue(2);

        connectedPin = nullptr;
        plugged = false;

        centerPoint = make_shared<Property<QPointF>>(QPointF(0,0), [this](QPointF oldV, QPointF newV){
            Q_UNUSED(oldV);
            _moveCenterTo(newV);
        });

        // opacity
        view_opacity = make_unique<QGraphicsOpacityEffect>();
        view_opacity->setOpacity(1);
        setGraphicsEffect(view_opacity.get());
    }

    LineMarker::LineMarker(WireCluster* wire, const Direction &direction): LineMarker(){
        owner = wire;
        domain = wire->getDomain();
        setSharedRenderer(domain->getMrkRenderer());
        centerP = boundingRect().bottomRight()/2;

        // validate direction
        if (domain->directional){
            if (direction == Direction::Uni){
                throw invalid_argument("Incompatible direction settings for domain and marker.");
            }
        }else{
            if (direction != Direction::Uni){
                throw invalid_argument("Incompatible direction settings for domain and marker.");
            }
        }
        this->direction = direction;

        // init lines
        line = make_unique<ConnectionLine>(this);
        line->setActive(false);
    }

    LineMarker::LineMarker(WireCluster* wire, XmlParser::XmlElement *data): LineMarker(){
        owner = wire;
        domain = wire->getDomain();
        setSharedRenderer(domain->getMrkRenderer());
        centerP = boundingRect().bottomRight()/2;

        // validate direction
        if (data->attributes.count(ATTR_LM_DIR)){
            direction = Domains::s2dir(data->attributes[ATTR_LM_DIR]);
            if (domain->directional){
                if (direction == Direction::Uni)
                    direction = Direction::Input;
            }else{
                if (direction != Direction::Uni)
                    direction = Direction::Uni;
            }
        }else{
            // none specified
            if (domain->directional)
                direction = Direction::Input;
            else
                direction = Direction::Uni;
        }

        // self position
        if (data->attributes.count("pos")){
            auto tmp = XmlParser::split(data->attributes["pos"], ";");
            //setPos(stod(tmp.at(0)), stod(tmp.at(1)));
            moveCenterTo(QPointF(stod(tmp.at(0)), stod(tmp.at(1))));
        }

        // init lines
        auto lineData = data->find(CL_TAG);
        if (lineData){
            line = make_unique<ConnectionLine>(this, lineData);
        }else{
            // no data, just create basic line
            line = make_unique<ConnectionLine>(this);
        }
        line->setActive(false);

        // plug
        if (auto c = data->find("ConnectTo")){
            if (auto e = owner->getSystem()->findElement(c->text)){
                // has connection
                if (c->attributes.contains("pin_idx")){
                    int idx = stoi(c->attributes["pin_idx"]);
                    if (auto p = e->getPin(idx)){
                        plugIn(p);
                    }
                }
            }
        }
    }

    LineMarker::~LineMarker(){
        unPlug();
        centerPoint->unbind();
        // remove itself from scene
        owner->getSystem()->_getScene()->removeItem(this);
    }

    void LineMarker::setOwner(WireCluster *wire){
        owner = wire;
    }

    Domain* LineMarker::getDomain(){
        return domain;
    }

    Direction LineMarker::getDirection(){
        return direction;
    }

    /**
     * push to front of marker
     */
    void LineMarker::toFront(){
        //raise();
        line->toFront();
    }

    void LineMarker::toBack() {
        //lower();
        line->toBack();
    }

    bool LineMarker::isPlugged(){
        return(plugged);
    }

    void LineMarker::_moveCenterTo(QPointF const& point){
        QPointF offset(boundingRect().width()/2, boundingRect().height()/2);
        QGraphicsSvgItem::setPos(point-offset);
    }

    void LineMarker::moveCenterTo(QPointF const& point){
        //_moveCenterTo(point);

        // update bindings
        centerPoint->setValue(point);
    }

    Cross* LineMarker::getAnchor(){
        return line->getStartMarker();
    }

    void LineMarker::inheritMouseMove(){
        mousePressed = true;
        grabMouse();
    }

    /**
     * @return the centerX
     */
    shared_ptr<Property<QPointF>> LineMarker::getBindPoint() {
        return centerPoint;
    }

    WireCluster* LineMarker::getWire() {
        return owner;
    }

    ConnectionLine* LineMarker::getLine(){
        return line.get();
    }

    ElementBase::Pin* LineMarker::getConnectedPin() {
        return connectedPin;
    }

    void LineMarker::plugIn(ElementBase::Pin* connectedPin) {
        if (plugged){
            //cerr << "Marker is already plugged into Pin!" << endl;
            return;
        }
        if (connectedPin->isConnected()){
            //cerr << "Pin is already plugged into Marker!" << endl;
            return;
        }
        if (!connectedPin->isCompatible(this)){
            //cerr << "Uncompatible pin-marker types!" << endl;
            return;
        }

        this->connectedPin = connectedPin;
        plugged = true;

        // Pin side
        connectedPin->_plugIn(this);
        
        // visuals
        view_opacity->setOpacity(0);
        line->setActive(true);
    }

    void LineMarker::unPlug(){
        if(!plugged)
            // already unplugged
            return;

        plugged = false;
        
        // Pin side
        connectedPin->unPlug();
        connectedPin = nullptr;

        // visuals
        line->setActive(false);
        view_opacity->setOpacity(1);
    }

    /**
     * clears centerPoint bindings
     */
    void LineMarker::unbind(){
        for(auto i=centerPoint->getBindingsLen(); i-->1;)
            centerPoint->unbind(i);
    }

    QList<QGraphicsItem*> LineMarker::getView(){
        QList<QGraphicsItem*> out;

        out.push_back(this);
        out.append(line->getView());

        return out;
    }

    XmlParser::XmlElement* LineMarker::to_xml(size_t objId){
        XmlParser::XmlElement* ret = new XmlParser::XmlElement(LM_TAG); // TODO const
        ret->attributes["id"] = to_string(objId);
        ret->attributes["pos"] = std::format("{};{}", centerPoint->getValue().x(), centerPoint->getValue().y());
        ret->attributes[ATTR_LM_DIR] = Domains::dir2s(direction);

        ElementBase::Element* elem = nullptr;
        int pinIndex=-1;
        if(connectedPin != nullptr) {
            elem = connectedPin->getOwner();
            pinIndex = elem->pinIndex(connectedPin);
        }

        XmlParser::XmlElement* node = new XmlParser::XmlElement("ConnectTo");
        if(elem==nullptr) {
            node->text = "null";
        }else{
            if(!elem->getSystem()->hasElement(elem)){
                ElementBase::Subsystem* sys2 = elem->getSystem();
                if(!elem->getSystem()->hasElement(elem->getSystem())){
                    throw runtime_error("Strange, connect to somewhere");
                }else{
                    node->text = sys2->getName() + "$" + elem->getName();
                }
            }else
                node->text = elem->getName();

            node->attributes["pin_type"] = connectedPin->getDomain()->typeName;
            node->attributes["pin_idx"] = to_string(pinIndex);
        }
        ret->append(node);

        ret->append(line->to_xml());

        return ret;
    }

    // EVENTS
    void LineMarker::mousePressEvent(QGraphicsSceneMouseEvent *event) {
        if (event->button() == Qt::LeftButton){
            offsetP = event->pos() - centerP;
            //drag0P = event->pos();
            mousePressed = true;
        }

        event->setAccepted(true);
    }

    void LineMarker::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
        if (mousePressed){
            if (plugged){
                auto pinView = connectedPin->getView();
                auto pluggedRect = pinView->mapRectToScene(pinView->boundingRect());
                if (!pluggedRect.contains(event->scenePos())){
                    // Left pin's geometry. Unplug
                    unPlug();
                    // start moving
                    QPointF target = event->scenePos() - offsetP;
                    moveCenterTo(target);
                }
            }else{
                auto pin = owner->getSystem()->_findPin(event->scenePos());
                
                if (pin && pin->isCompatible(this)){
                    plugIn(pin);
                }else{
                    // no pin, just move the marker
                    QPointF target = event->scenePos() - offsetP;
                    moveCenterTo(target);
                }
            }
            event->setAccepted(true);
        }
    }

    void LineMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        mousePressed = false;
        ungrabMouse();
        event->setAccepted(true);
    }

    void LineMarker::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
        Q_UNUSED(event);
        hovering = true;

        update();
    }
    void LineMarker::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
        Q_UNUSED(event);
        hovering = false;

        update();
    }

    void LineMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        if (hovering){
            painter->setPen(QPen(QBrush(QColor::fromString("aqua")), 1));
            auto r = renderer()->viewBoxF().adjusted(-1,-1,0,0);
            painter->drawRect(r);
        }

        QGraphicsSvgItem::paint(painter, option, widget);
    }
}
