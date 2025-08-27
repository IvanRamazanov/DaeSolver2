#ifndef EBASE_ELEM_LIB_WIDGET_H
#define EBASE_ELEM_LIB_WIDGET_H

#include <QGraphicsDropShadowEffect>
#include <QWidget>
#include <QPoint>
#include <QPointF>
#include <QVBoxLayout>
#include <QLabel>
#include "ElemBaseDecl.h"

namespace ElementBase{
    class ElementWidgetLib : public QWidget{
        Q_OBJECT

        QGraphicsDropShadowEffect *selectedEffect;
        QPoint dragAnchor;

        protected:
            Element *owner;

            //virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        public:
            ElementWidgetLib(Element *owner);

        protected:
            // events
            virtual void mouseMoveEvent(QMouseEvent *event) override;
            virtual void mousePressEvent(QMouseEvent *event) override;
            virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
            virtual void mouseReleaseEvent(QMouseEvent *event) override;

        friend class Element;
        friend class View;
    };
}

#include "Element.h"

#endif
