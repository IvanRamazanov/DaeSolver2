#ifndef EBASE_PIN_H
#define EBASE_PIN_H

#include "ElemBaseDecl.h"

#include <QGraphicsSvgItem>
#include "../MathPack/Domain.h"
#include "../MathPack/Parser.h"
#include "../Connections/Property.h"

namespace ElementBase{
    class PinWidget : public QGraphicsSvgItem {
        Q_OBJECT

        public:
            PinWidget(Pin *owner);
            ~PinWidget();

        private:
            Pin* owner;

        protected:
            // drag'n'drop
            QPointF dragStartPosition;
            bool mouseWasPressed = false;

            // events
            virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
            virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
            //virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
            //virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;

        friend class Pin;
    };

    class Pin{
        private:
            Element *owner;
            string name;
            PinWidget *view = nullptr;
            Connections::LineMarker *connectedMarker = nullptr;
            bool connected = false,
                is_external = false,
                is_hidden = false; // primarily for compount elements to hide internal connections
            QPointF offset, position; // pos offset (local)

        protected:
            Domains::Domain* domain;
            Domains::ConnDirection direction = Domains::Uni;

            // store global position coordinates
            shared_ptr<Connections::Property<QPointF>> centerPoint; // in scene coords!

        public:
            const Domains::Connector connector;
            // discrete pins
            bool is_cached = false; // if true, will cache previous step value (to break alg. loops)
            double *data;
            size_t dataSize=1; // only for discrete connections
            size_t dataSizeSetting=-1; // default size
            
            
        protected:
            virtual void dragDetected(QPointF const& pos);

        public:
            Pin(string const& name, Element* owner, QPointF const& pos, Domains::Domain *domain, bool is_external=false, Domains::ConnDirection direction=Domains::Uni);
            Pin(Element* owner, XmlParser::XmlElement *info);
            ~Pin();

            const string& getName();

            /**
             * removes connection
             */
            void unPlug();

            /**
             * Should be called only from LineMarker!
             */
            void _plugIn(Connections::LineMarker* marker);

            /**
             * @param marker
             * @returns whether marker can be connected (plugged) to this pin
             */
            bool isCompatible(Connections::LineMarker* marker);

            shared_ptr<Connections::Property<QPointF>> getBindPoint();

            Connections::LineMarker* getConnectedMarker();

            bool isConnected();

            bool isExternal();

            Element* getOwner();

            Domains::Domain* getDomain();

            size_t getDataSize();

            const Domains::ConnDirection& getDirection();

            void toFront();

            QGraphicsItem* getView();

            void moveCenterTo(QPointF newPos);
            void updatePosition();

        friend class PinWidget;
    };
}

#endif
