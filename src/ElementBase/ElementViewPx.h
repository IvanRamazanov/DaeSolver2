#ifndef EBASE_ELEMENT_VIEWPX_H
#define EBASE_ELEMENT_VIEWPX_H

namespace ElementBase{
    class Element;
}

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <string>
#include "Element.h"

using namespace std;

namespace ElementBase{
    class Element;
    /**
     * Container that draws Elements graphics (image)
     * TODO: SVG?
     */
    class View : public QGraphicsPixmapItem {
        public:
            View(Element *owner);

            void setHeight(int value);

            using QGraphicsPixmapItem::scale;
            using QGraphicsPixmapItem::boundingRect;

        private:
            Element *owner;
            QString image;
            int height = 100;

            // drag event
            QPointF eventOffset;
            bool readyToMove=false, readyToDrag=false;

            void initDND();

        protected:
            // events
            virtual void keyReleaseEvent(QKeyEvent *event) override;
            virtual void keyPressEvent(QKeyEvent *event) override;
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    };
}

#endif
