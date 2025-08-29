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

#include "Pin.h"
#include "../Connections/Wire.h"
#include <QGraphicsSceneHoverEvent>
#include <QTimer>

constexpr auto PIN_DIR = "direction";
constexpr auto ATTR_NAME = "name";
constexpr auto ATTR_DOMAIN = "domain";
constexpr auto ATTR_EXTERNAL = "external";

using namespace std;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;

namespace ElementBase{
    PinWidget::PinWidget(Pin *owner): owner(owner){
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        setSharedRenderer(owner->domain->getPinRenderer());
        setPos(owner->position);
        setCursor(Qt::PointingHandCursor);

        offset = boundingRect().bottomRight()/2; // center point of pin's graphics

        // enable D'n'D
        //setAcceptDrops(true);
        //setAcceptHoverEvents(true);
    }

    PinWidget::~PinWidget(){
        //owner->view = nullptr;
        //qDebug() << "PinWidget destroyed";
    }

    /*
     * ___EVENT HANDLERS___
     */

    void PinWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
        //setEffect(new DropShadow(BlurType.GAUSSIAN, Color.AQUA, 2, 1, 0, 0));
        //setCursor(Qt::PointingHandCursor);
        cout << "Pin hover enter" << endl;
        QGraphicsSvgItem::hoverEnterEvent(event);
    }

    void PinWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
        //getView().setEffect(null);
        //setCursor(Qt::ArrowCursor);

        if(mouseWasPressed){
            cout << "Pin hover leave" << endl;
            mouseWasPressed = false;
            owner->dragDetected(event->scenePos());
        }

        QGraphicsSvgItem::hoverLeaveEvent(event);
    }

    // anchor for d'n'd
    void PinWidget::mousePressEvent(QGraphicsSceneMouseEvent *event){
        dragStartPosition = event->pos();
        mouseWasPressed = true;

        event->setAccepted(true);
    }

    QVariant PinWidget::itemChange(GraphicsItemChange change, const QVariant &value){
        if (change == ItemSceneHasChanged && scene()) {
            QTimer::singleShot(0, [this](){
                owner->centerPoint->setValue(mapToScene(offset));
            });
        }else if(change == ItemPositionHasChanged){
            owner->centerPoint->setValue(mapToScene(offset));
        }

        return QGraphicsSvgItem::itemChange(change, value);
    }



    /**
     * Detects drag'n'drop
     */
    void PinWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
        if(!mouseWasPressed){
            return;
        }
        if ((event->pos() - dragStartPosition).manhattanLength() < 1)
            return;

        mouseWasPressed = false;
        owner->dragDetected(event->scenePos());
    }

    void PinWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
        Q_UNUSED(event);
        mouseWasPressed = false;
    }
    /* END */


    Pin::Pin(string const& name, Element* owner, QPointF const& pos, Domain *domain, bool is_external, Domains::ConnDirection direction):
        owner(owner),
        name(name),
        is_external(is_external),
        position(pos),
        domain(domain),
        connector(domain->getConnector(name))
    {
        // validate
        if (domain->directional){
            if (direction == Domains::ConnDirection::Uni)
                throw invalid_argument("Pin::Unidirectional pin is not allowed in directional domain");
        }else{
            if (direction != Domains::ConnDirection::Uni)
                throw invalid_argument("Pin::Directional pin is not allowed in unidirectional domain");
        }

        this->direction = direction;

        // cached?

        // set pos
        centerPoint = make_shared<Connections::Property<QPointF>>(pos);
        // graphics
        view = new PinWidget(this);
    }

    Pin::Pin(Element* owner, XmlParser::XmlElement *info): 
        owner(owner),
        name(info->attributes.contains(ATTR_NAME) ? info->attributes[ATTR_NAME] : throw runtime_error("Invalid pin definition: name is requred!")),
        domain(info->attributes.contains(ATTR_DOMAIN) ? Domains::Domain::getDomain(info->attributes[ATTR_DOMAIN]) : throw runtime_error("Invalid pin definition: domain is requred!")),
        connector(domain->getConnector(name))
    {
        // direction
        if (domain->directional){
            if (info->attributes.count(PIN_DIR)){
                direction = Domains::s2dir(info->attributes[PIN_DIR]);
                if (direction == Domains::ConnDirection::Uni){
                    throw runtime_error("Invalid pin definition: Unidirection is set for directional pin!");
                }
            }else{
                throw runtime_error("Invalid pin definition: direction is requred for directional domains!");
            }
        }

        // chache
        if (info->attributes.contains("chached")){
            is_cached = XmlParser::stob(info->attributes["chached"]);
        }

        // pass through
        if (info->attributes.contains(ATTR_EXTERNAL)){
            is_external = XmlParser::stob(info->attributes[ATTR_EXTERNAL]);
        }

        // set pos
        if (info->attributes.contains("x")){
            position.setX(stod(info->attributes["x"]));
        }
        if (info->attributes.contains("y")){
            position.setY(stod(info->attributes["y"]));
        }
        centerPoint = make_shared<Connections::Property<QPointF>>(position);
        // layout
        view = new PinWidget(this);
    }

    void Pin::_plugIn(Connections::LineMarker* marker) {
        if (connected){
            return;
        }

        // bind
        centerPoint->bind(marker->getBindPoint());
        
        // update position. Initial position will be wrogn because of how the scene is drawn...
        //updatePosition();

        connected = true;
        this->connectedMarker = marker;
        view->setVisible(false);
    }

    /**
     * Unplug element pin
     */
    void Pin::unPlug(){
        if (!connected)
            return;
        
        connected = false;
        // unbind graphics
        centerPoint->unbind();

        connectedMarker->unPlug();
        connectedMarker=nullptr;

        //if(view) // may be null during destruction
            view->setVisible(true);
    }

    Pin::~Pin(){
        unPlug();
    }

    const string& Pin::getName(){
        return name;
    }

    Connections::LineMarker* Pin::getConnectedMarker() {
        return connectedMarker;
    }

    bool Pin::isConnected(){
        return connected;
    }

    bool Pin::isExternal(){
        return is_external;
    }

    Element* Pin::getOwner() {
        return owner;
    }

    void Pin::toFront(){
        if (!is_external){
            owner->toFront();
        }
        
            //view->setZValue(0); // TODO cycle through scene
        
    }
    
    bool Pin::isCompatible(Connections::LineMarker* marker){
        return domain->typeName == marker->getDomain()->typeName &&
                direction == marker->getDirection();
    }

    Domain* Pin::getDomain(){
        return domain;
    }

    const Direction& Pin::getDirection(){
        return direction;
    }

    shared_ptr<Connections::Property<QPointF>> Pin::getBindPoint(){
        return centerPoint;
    }

    void Pin::moveCenterTo(QPointF newPos){
        view->setPos(newPos-view->offset);
        centerPoint->setValue(newPos);
    }

    /**
     * Starts Drag'n'Drop sequence; creates Marker (Wire)
     */
    void Pin::dragDetected(QPointF const& pos){
        if (connectedMarker == nullptr) {
            // update position. Initial position will be wrogn because of how the scene is drawn...
            //updatePosition();
            // create wire
            auto sys = is_external ? owner->getSystem()->getSystem() : owner->getSystem();
            Connections::WireCluster* wire = sys->addWire(domain);
            auto activeMarker = wire->initConnection(this);

            activeMarker->moveBy(pos.x(), pos.y());
            activeMarker->inheritMouseMove();
        } else {
            auto tmp = connectedMarker;
            unPlug();
            tmp->inheritMouseMove();
        }
    }

    size_t Pin::getDataSize(){
        return dataSize;
    }

    QGraphicsItem* Pin::getView(){
        return view;
    }

    void Pin::updatePosition(){
        centerPoint->setValue(view->mapToScene(view->offset));
    }

}
