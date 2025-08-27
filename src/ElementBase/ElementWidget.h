#ifndef EBASE_ELEM_WIDGET2_H
#define EBASE_ELEM_WIDGET2_H

#include "ElemBaseDecl.h"

#include <QVBoxLayout>
#include <QGraphicsWidget>
#include <QWidget>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QGraphicsProxyWidget>

namespace ElementBase{
    class View;

    class ElemBodyWrapper : public QGraphicsWidget{
        Element *owner;

        public:
            QRectF realSize;

        public:
            View *view = nullptr;

            ElemBodyWrapper(Element *owner);

            void addPin(Pin* pin);

    };

    class ElementWidget : public QGraphicsWidget{
        ElemBodyWrapper *elemBody;
        QLabel *nameLabel;
        QGraphicsDropShadowEffect *selectedEffect;

        protected:
            Element *owner;
            virtual QMenu* initMenu();
            void showRenameWindow();

            //QRectF boundingRect() const override;

            //virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        public:
            ElementWidget(Element *owner);
            ~ElementWidget();

            void setRotation(qreal angle);

            void addPin(Pin* pin);

            double getBodyHeight();

        friend class Element;
        friend class View;
    };
    
}

#include "Element.h"

#endif
