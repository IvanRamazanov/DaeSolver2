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

#ifndef CONN_WIRECLUSTER_H
#define CONN_WIRECLUSTER_H

#include <QWidget>
#include <QPainter>
#include <QColor>
#include <QGraphicsLineItem>
#include <QLineF>
#include <QGraphicsSceneMouseEvent>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <cmath>
#include <algorithm>
#include <variant>
#include "../MathPack/Parser.h"
#include "../MathPack/Domain.h"
#include "../ElementBase/Element.h"
#include "../ElementBase/Subsystem/Subsystem.h"
#include "../MathPack/Vectors.h"
#include "Property.h"


namespace Connections{
    class ConnectionLine;
    class LineMarker;
    class WireCluster;

    constexpr auto WIRE_TAG = "Wire";
    constexpr auto LM_TAG = "SubWire";
    constexpr auto CL_TAG = "Line";
    constexpr auto ATTR_DOMAIN = "domain";
    constexpr auto ATTR_LM_DIR = "direction";
    constexpr auto LINE_POS_TAG = "StartEnd";

    class Cross : public QGraphicsEllipseItem {
        //Q_OBJECT

        private:
            ConnectionLine* owner;
            Domains::Domain *domain;
            shared_ptr<Property<QPointF>> centerPoint;
            variant<Cross*, LineMarker*> bindedTo;
            bool binded = false;

            // movement
            QPointF anchorPoint;
            bool mousePressed = false;

        protected:
            

            void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
            void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
            void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        public:
            Cross(ConnectionLine* owner);
            Cross(ConnectionLine* owner, QPointF const& position);
            ~Cross();

            /**
             * clears centerPoint bindings
             */
            void unbind();

            void bindToCross(Cross* other);

            void bindToMarker(LineMarker* master);

            template <typename T>
            T* getBinded();

            void moveCenterTo(QPointF const& point);

            void toFront();

        friend class ConnectionLine;
        friend class WireCluster;
    };

    class LineMarker : public QGraphicsSvgItem {
        Q_OBJECT

        private:
            WireCluster* owner;
            Domains::Domain* domain;
            Domains::ConnDirection direction;
            // move events
            QPointF centerP // center of SvgGraphics (local coords)
                    ,offsetP // offset from center of this Graphics to mouse press event (for movement)
                    //,drag0P
                    ; 
            bool mousePressed = false,
                hovering = false;
            unique_ptr<QGraphicsOpacityEffect> view_opacity;

        protected:
            unique_ptr<ConnectionLine> line;
            ElementBase::Pin* connectedPin;
            // shared_ptr<Property<int>>   bindX,
            //                             bindY;
            shared_ptr<Property<QPointF>> centerPoint; // scene coords; for binding
            bool plugged;


        private:
            // just the move, to prevent potential loop
            void _moveCenterTo(QPointF const& point);

        protected:
            LineMarker();

            // events
            virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
            virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
            //virtual void dragEnterEvent(QDragEnterEvent *event) override;
            //virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
            virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
            
        public:
            LineMarker(WireCluster* wire, const Domains::ConnDirection &direction = Domains::Uni);

            /**
             * Restore state from save file
             */
            LineMarker(WireCluster* wire, XmlParser::XmlElement *data);
            ~LineMarker();

            void setOwner(WireCluster *wire);

            Domains::Domain* getDomain();

            Domains::ConnDirection getDirection();

            /**
             * push marker to front
             */
            void toFront();

            void toBack();

            bool isPlugged();

            void inheritMouseMove();

            /**
             * TODO ???
             */
            //bool isCompatible(LineMarker* other);

            //void move(int x, int y, bool global_coord = false);
            void moveCenterTo(QPointF const& point);

            /**
             * @return the centerX
             */
            shared_ptr<Property<QPointF>> getBindPoint();

            WireCluster* getWire();

            ConnectionLine* getLine();

            /**
             * @returns Cross of its Line
             */
            Cross* getAnchor();

            ElementBase::Pin* getConnectedPin();

            void plugIn(ElementBase::Pin* connectedPin);

            void unPlug();

            /**
             * clears centerPoint bindings
             */
            void unbind();

            QList<QGraphicsItem*> getView();

            XmlParser::XmlElement* to_xml(size_t objId=0);

        friend class WireCluster;
    };

    class Line : public QGraphicsLineItem{
        private:
            bool horizontal;
        protected:
            ConnectionLine* owner;
            // drag'n'drop
            QPointF dragStartPosition;
            bool mouseWasPressed = false;



        protected:
            // events
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
            virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
            virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
            virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
            // virtual void focusInEvent(QFocusEvent* event) override;
            // virtual void focusOutEvent(QFocusEvent* event) override;
            virtual void keyReleaseEvent(QKeyEvent *event) override;

            void dragDetected(QGraphicsSceneMouseEvent *event);

        public:
            /**
             *
             * @param sX
             * @param sY
             * @param eX
             * @param eY
             * @param horizon
             */
            Line(ConnectionLine* owner, double sX, double sY, double eX, double eY, bool horizontal);
            ~Line();

            /**
             * @return the horizontal
             */
            bool isHorizontal();

            void setActive(bool isActive, QColor const& activeColor);

        friend class ConnectionLine;
    };

    class ConnectionLine {
        private:
            Domains::Domain* domain;
            shared_ptr<Property<QPointF>> startPoint, endPoint;
            bool isActive = true; // line appearance (plugged or not)
            QColor color;

        protected:
            WireCluster* owner;
            variant<LineMarker*, unique_ptr<Cross>> marker;
            deque<unique_ptr<Line>> lines;
            QList<double> linePositions;
            unique_ptr<Cross> startMarker;


        private:
            void init(XmlParser::XmlElement *data=nullptr);

            /**
             * Redraw lines in vicinity of lineIdx
             */
            void redrawLines(size_t lineIdx);

        protected:
            /**
             * move selected line to new coordinates (and preserve vertical/horizontal orientation)
             */
            void shiftLine(Line *line, QPointF const& newPos);

            /**
             * move the very end of first or last line
             * @param firstLine true -move starting point; false - ending point
             */
            void moveEndTo(QPointF const& newPos, bool firstLine);
    
        public:
            /**
             * @param data for loading from save file
             */
            ConnectionLine(LineMarker* anchor, XmlParser::XmlElement *data=nullptr);

            // For CrossToCross lines
            ConnectionLine(WireCluster *owner);
            ~ConnectionLine();

            void setVisible(bool val);

            void toFront();

            void toBack();

            WireCluster* getWire();

            void setActive(bool val);

            bool isCrossToCross();

            QList<QGraphicsItem*> getView();

            /**
             * @return the crossMarker
             */
            Cross* getStartMarker();

            template<typename T>
            T getEndMarker();

            XmlParser::XmlElement* to_xml();

        friend class Line;
    };

    class WireCluster{
        private:
            struct WireConsumeData{
                size_t index;
                string class_name;
                vector<string> consumed_wire_data;
                vector<string> master_wire_data;
            };

            Domains::Domain *domain = nullptr;
            vector<unique_ptr<LineMarker>> lineMarkers;
            vector<unique_ptr<ConnectionLine>> ContContList;
            vector<vector<Cross*>> dotList;
            static unique_ptr<WireConsumeData> wireConsumptionBackup;
            weak_ptr<Cross> eventCross;
            ElementBase::Subsystem *system = nullptr;
            static LineMarker* activeWireConnect;

            // discrete wires
            double* data; // point to output Pin data field
            size_t dataSize;


        private:
            /**
             * Binds all crosses in dotList to first Cross in each line
             * @param masterMarker in case of 2 marker Wire, 
             */
            void bindCrosses(); // LineMarker *masterMarker=nullptr ??

        public:

            /**
             * Create basic wire: two markers
             */
            WireCluster(ElementBase::Subsystem *system, Domains::Domain *domain);
            WireCluster(ElementBase::Subsystem *system, XmlParser::XmlElement *xml_data);

            /**
             * Sets up first wire connection (for basic wire).
             * @returns unconnected marker
             */
            LineMarker* initConnection(ElementBase::Pin* pin);

            XmlParser::XmlElement* to_xml();

            Domains::Domain* getDomain();
            
            static LineMarker* getActiveMarker();
            static void setActiveMarker(LineMarker* value);

            pair<size_t, size_t> findDot(Cross* element);

            ElementBase::Subsystem* getSystem();
            void setSystem(ElementBase::Subsystem* sys);

            /**
             * Merge wires (usually during LineMarker dragging)
             */
            void consumeWire(ConnectionLine* eventSource);

            LineMarker* getMarker(size_t idx);
            LineMarker* findMarker(Domains::ConnDirection dir);
            const vector<unique_ptr<LineMarker>>& getMarkers();

            QList<QGraphicsItem*> getView();

            /**
             *  Amount of markers (size of the wire)
             * @return
             */
            size_t size();

            /**
             * for d'n'd event. Creates new marker at (x,y) rooting from line
             * @param line
             * @param x
             * @param y
             */
            LineMarker* addLineMarker(ConnectionLine* line, int x, int y);
            
            /**
             * create marker and plug it into pin
             */
            LineMarker* addLineMarker(ElementBase::Pin *pin);

            /**
             * Delete event. Removes selected line (and marker) from the cluster.
             */
            void remove(ConnectionLine* line);


            // discrete sys API

            double* getDataRef();
            size_t getDataSize();
            /**
             * Note: Wire doesn't own the data. Data is located in Workspace
             */
            void initDataRef(double *dataRef, size_t size);
    };
}

#endif
