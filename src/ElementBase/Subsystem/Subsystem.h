#ifndef EBASE_SUBSYS_H
#define EBASE_SUBSYS_H

#include "../ElemBaseDecl.h"

#include <vector>
#include <string>
#include <memory>
#include <limits>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QFileDialog>
#include "../../MathPack/Parser.h"
#include "../../MathPack/Domain.h"
#include "../Element.h"

using namespace std;

namespace ElementBase{
    class SelectionModel : public QGraphicsObject {
        Q_OBJECT

        private:
            vector<Element*> selection; // technically, could be any graphics object, but for now limit only to elements
            vector<Element*> selectedElements;

        protected:
            virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

        public:
            double ix,iy;
            QRect rect;

            SelectionModel();

            void init(double x, double y);

            void update(double x, double y);

            void confirm(vector<unique_ptr<Element>> const& elements);

            void clear();

            vector<Element*> getSelectedElements();

            QRectF boundingRect() const override;
    };

    class Stage : public QGraphicsView{
        Q_OBJECT

        unique_ptr<SelectionModel> selectionModel = nullptr;
        Subsystem *owner = nullptr;
        QGraphicsScene *scene = nullptr;
        bool selectionInProgress = false;

        public:
            Stage(Subsystem *owner);
            ~Stage();

        protected:
            virtual void keyReleaseEvent(QKeyEvent *keyEvent) override;
            virtual void mousePressEvent(QMouseEvent *mouseEvent) override;
            //virtual void mouseMoveEvent(QMouseEvent *event) override;
            virtual void mouseReleaseEvent(QMouseEvent *event) override;
            virtual void dropEvent(QDropEvent *event) override;
            virtual void dragEnterEvent(QDragEnterEvent *event) override;
            //virtual void resizeEvent(QResizeEvent *event) override;

        friend class Subsystem;
    };

    class Subsystem : public Element {
        private:
            class SystemView : public ElementWidget{
                public:
                    SystemView(Element* owner);

                protected:
                    virtual QMenu* initMenu() override;
            };
            
            Stage *drawingBoard = nullptr;
            vector<unique_ptr<Element>> elementList;
            vector<unique_ptr<Connections::WireCluster>> wireList;
            int windowHeight = 500,
                windowWidth = 400;

            void initStage();
            void removeElement(size_t idx);

        public:
            Subsystem();
            ~Subsystem();

            ElemTypes type() override;

            void setName(string const& name) override;

            /**
             * Checks if elemName is free to use.
             * If necessary, creates unique name within this subsystem
             */
            string getUniqueName(string const& elemName);

            void setSystem(Subsystem *sys) override;
            void setSystem(Subsystem *oldSys, vector<unique_ptr<Element>> children);

            void loadState(XmlParser::XmlElement *elemInfo) override;
            XmlParser::XmlElement* to_xml() override;
            
            /**
             * Clear subsystem.
             * Removes all elements and wires
             */
            void clear();

            QWidget* openDialogStage() override;

            void linkBottomBar(QObject *parentBar);

            void initView() override;

            /**
             * Add external pin of child element
             * @param pin - child's external pin
             * @param leftSide - location, where to put the pin (left/right side of Subsys view)
             */
            void addPinView(shared_ptr<Pin> pin, bool leftSide);

            void compileTo();

            void addElement(Element *elem, QPointF const& atPoint=QPointF());
            void removeElement(Element *item);
            vector<Element*> getElements(bool recursive=false);
            
            /**
             * Shallow search
             */
            bool hasElement(Element* elem);

            Element* findElement(string const& name);

            /**
             * Recursively, all wires
             */
            vector<Connections::WireCluster*> getWires();

            Connections::WireCluster* addWire(Domains::Domain *wireDomain);
            void removeWire(Connections::WireCluster* wire);

            // utils
            QGraphicsScene* _getScene();
            Pin* _findPin(QPointF atPos);
            Connections::ConnectionLine* _findLine(QPointF atPos);

        friend class Stage;
    };
}

#include "../ElementFactory.h"
#include "../../Connections/Wire.h"
#include "../Pass.h"

#endif
