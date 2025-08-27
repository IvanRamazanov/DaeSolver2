#include "ElementViewPx.h"

namespace ElementBase{
    View::View(Element *owner): owner(owner){
        image = QString::fromStdString(owner->imgPath);
        setPixmap(QPixmap(image));
        setScale(0.2);

        //view.setEffect(new DropShadow(BlurType.GAUSSIAN, Color.AQUA, 1.0, 1.0, 0, 0));
        setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    }

    void View::initDND(){
        
    }

    void View::setHeight(int value){
        Q_UNUSED(value);
        // TODO do something about this function (implement or remove)

        // image = image.scaledToHeight(value);
        // setMinimumSize(image.size());
        // update();
    }

    // Events

    void View::mousePressEvent(QGraphicsSceneMouseEvent *event) {
        if (readyToMove || readyToDrag){
            // ignore any new inputs
            event->setAccepted(true);
            return;
        }

        if (event->button() == Qt::LeftButton){
            readyToMove = true;
        }else if(event->button()==Qt::RightButton && event->modifiers() & Qt::Key_Control){
            readyToDrag = true;
        }else{
            return;
        }

        eventOffset = event->scenePos() - owner->_view->scenePos();
        setSelected(true);
        setFocus(Qt::MouseFocusReason);
        event->setAccepted(true);
    }

    // void View::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    //     auto newPos = initialPoint + event->pos() - anchorPoint;
    //     if(newPos.x() < 0)
    //         newPos.setX(0);
    //     if(newPos.y() < 0)
    //         newPos.setY(0);
    //     setPos(newPos);
    // }

    void View::keyReleaseEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Delete){
            owner->getSystem()->removeElement(owner);
            event->setAccepted(true);
        }else{
            event->setAccepted(false);
        }
    }

    void View::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
        if(readyToDrag){
            // start D'n'D
            QDrag *db = new QDrag(event->widget());
            ElementMime *content = new ElementMime();
            content->setData(CUSTOM_FORMAT, owner);
            db->setMimeData(content);
            QPixmap img = QPixmap(image).scaled(img.width()/5,img.height()/5);
            db->setPixmap(img);

            db->exec();         
        }else if(readyToMove){
            // just move
            owner->moveTo(event->scenePos()-eventOffset);
        }else{
            return;
        }
        event->setAccepted(true);
    }

    void View::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
        readyToDrag = false;
        readyToMove = false;

        if(event->button() == Qt::RightButton){
            auto menu = owner->_view->initMenu();
            menu->move(event->screenPos());
            menu->exec();
            delete menu;
        }
    }

    void View::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
        if (event->button() == Qt::LeftButton){
            auto wind = owner->openDialogStage();
            wind->show();
        }
    }

    void View::keyPressEvent(QKeyEvent *event){
        if(event->key()==Qt::Key_Left){
            owner->moveBy(-1, 0);
            event->setAccepted(true);
        }else
        if(event->key()==Qt::Key_Right){
            owner->moveBy(1, 0);
            event->setAccepted(true);
        }else
        if(event->key()==Qt::Key_Up){
            owner->moveBy(0, -1);
            event->setAccepted(true);
        }else
        if(event->key()==Qt::Key_Down){
            owner->moveBy(0, 1);
            event->setAccepted(true);
        }else
        if(event->key()==Qt::Key_R && (event->modifiers() & Qt::ControlModifier)){
            owner->rotate();
            event->setAccepted(true);
        }else{
            //QGraphicsPixmapItem::keyPressEvent(event);
            event->setAccepted(false);
        }
    }

}