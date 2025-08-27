/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Wire.h"

using namespace std;

namespace Connections{
    /**
     *
     * @author Ivan
     */

    constexpr double CROSS_H = 6;

    Cross::Cross(ConnectionLine* owner): owner(owner){
        domain = owner->getWire()->getDomain();

        setVisible(true);
        setZValue(1);
        setRect(0, 0, CROSS_H, CROSS_H);
        setPen(QPen(QBrush(), 0));
        setBrush(QBrush(domain->wireColour));
        
        //owner->getWire()->getSystem()->getDrawBoard().getChildren().add(this);

        centerPoint = make_shared<Property<QPointF>>(QPointF(), [this](QPointF oldV, QPointF newV) -> void {
            Q_UNUSED(oldV);
            this->setPos(newV.x()-CROSS_H/2, newV.y()-CROSS_H/2);
        });
    }

    Cross::Cross(ConnectionLine* owner, QPointF const& position): Cross(owner){
        centerPoint->setValue(position);
    }

    Cross::~Cross(){
        centerPoint->unbind();
        // remove itself from scene
        owner->getWire()->getSystem()->_getScene()->removeItem(this);
    }
 
    void Cross::mousePressEvent(QGraphicsSceneMouseEvent *event){
        anchorPoint = event->pos();
        mousePressed = true;
        event->setAccepted(true);
    }

    void Cross::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        mousePressed = false;
        event->setAccepted(true);
    }

    void Cross::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
        if (mousePressed){
            moveCenterTo(event->scenePos()-anchorPoint);

            event->setAccepted(true);
        }
    }

    void Cross::unbind(){
        // to prevent losing Line binding, unbing everything except 0-th (Line)
        for(auto i=centerPoint->getBindingsLen(); i-->1;){
            centerPoint->unbind(i);
        }

        // if (std::holds_alternative<Cross*>(bindedTo)){
        //     std::get<Cross*>(bindedTo)->centerPoint->unbind(centerPoint.get());
        // }else if (std::holds_alternative<LineMarker*>(bindedTo)){
        //     std::get<LineMarker*>(bindedTo)->getBindPoint()->unbind(centerPoint.get());
        // }

        setVisible(true);

        binded = false;
    }

    void Cross::bindToCross(Cross* other){
        if(binded){
            unbind();
        }

        other->centerPoint->bind(centerPoint);
        bindedTo = other;

        // visuals
        setVisible(false);

        binded = true;
    }

    void Cross::bindToMarker(LineMarker* master){
        if(binded){
            unbind();
        }

        master->getBindPoint()->bind(centerPoint);
        bindedTo = master;

        //visuals
        setVisible(false);

        binded = true;
    }

    template <typename T>
    T* Cross::getBinded(){
        return get<T*>(bindedTo);
    }

    void Cross::moveCenterTo(QPointF const& point){
        centerPoint->setValue(point);
    }

    void Cross::toFront(){
        // TODO cycle through Scene
        //setZValue(0);
    }
}
