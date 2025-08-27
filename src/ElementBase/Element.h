/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#ifndef EBASE_ELEMENT_H
#define EBASE_ELEMENT_H

#include "ElemBaseDecl.h"

#include <string>
#include <vector>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QImage>
#include <QComboBox>
#include <QGraphicsLinearLayout>
#include <QGraphicsOpacityEffect>
#include <QPointF>
#include <QPoint>
#include <QPainter>
#include <QLineEdit>
#include <QString>
#include <QVariant>
#include <QFont>
#include <QMenu>
#include <QMimeData>
#include <QFile>
#include <format>
#include <QRegularExpression>

#include "../MathPack/Parser.h"
#include "../MathPack/Domain.h"
#include "ElementViewPx.h"
#include "ElementWidget.h"
#include "ElementLibWidget.h"
#include "Parameters/Parameter.h"
#include "../MathPack/WorkSpace.h"
#include "../MathPack/StringGraph.h"


namespace ElementBase{
    enum ElemTypes {Elem, Subsys};

    class ElementMime : public QMimeData{
        Q_OBJECT

        private:
            Element* customData;

        public:
            void setData(const QString &mimeType, Element* data);

            bool hasElement();

            virtual bool hasFormat(const QString &mimeType) const override;

            Element* elementData() const;
    };

    struct GraphFlowData{
        bool is_reference=false;
        Domains::Domain *dom = nullptr;
        std::set<Pin*> pins;
        std::set<std::string> varNames;

        GraphFlowData();
        GraphFlowData(initializer_list<Pin*> vec);
        GraphFlowData(vector<std::string> varNamesVec, vector<Pin*> vec);

        void addPin(Pin *pin);
        void addVar(string const& var, Pin *pin);
        /**
         * sets is_reference flag and makes some checks
         */
        void setReference();
        bool isEmpty();
        void verifyDomain(Pin *pin);
    };

    /**
     *
     * @author Ivan
     */
    class Element{
        private:
            Subsystem *system = nullptr;
            
        protected:
            string id;
            string name;
            string imgPath;
            string imgName;
            string description;
            double  contStep=15,
                    maxX;
            vector<shared_ptr<Pin>> pins; // shared because can be moved to Subsystem layout
            vector<unique_ptr<Parameter>> parameters;
            vector<InitParam> stateInitValues;
            vector<MathPack::StringGraph> equations;

            // connector var name + it's domain
            // TODO: replace with StringGraph? For complex cross domain flows?
            vector<GraphFlowData> flows;
            string discreteFunction = "";
            WorkSpace::WorkSpace workspace;
            vector<Pin*> leftSidePins, rightSidePins;
            bool discreteFlag = false,
                physicalFlag = false;
            
            // layout
            double x=0, y=0;
            int rot=0;
            
        public:
            ElementWidget *_view = nullptr;


        private:
            Pin* _addPin(string const& name, Domains::Domain *dom, bool isExternal, Domains::ConnDirection direction);

        protected:
            Element();

            string getDescription();

            /**
             * factory for parameters
             */
            void addParameter(XmlParser::XmlElement *info);
            void addParameter(string const& name, string const& title, string const& value, ParamType pType=ParamType::Scalar);
            Parameter* addScalarParameter(string const& name, string const& title, double value);
            Parameter* addVectorParameter(string const& name, string const& title, vector<double> const& value);

            void addInitValue(XmlParser::XmlElement *info, WorkSpace::DifferentialVar *var);
            void addInitValue(string const& name, string const& title, vector<double> const& value, WorkSpace::DifferentialVar *var);

            Pin* addPin(string const& name, QPointF const& pinPos, Domains::Domain *dom, bool isExternal=false, Domains::ConnDirection direction=Domains::Uni);
            Pin* addPin(string const& name, Domains::ConnDirection direction);
            void alignPin(Pin *pin, bool leftSide);

            void initPortSize(Pin *pin, size_t size, Pin* verifyMatch=nullptr);

        public:
            /**
             * @param path - path to element spec file (resourse or custom)
             */
            Element(filesystem::path path);
            virtual ~Element();

            virtual ElemTypes type();

            void initFlags();

            virtual void loadState(XmlParser::XmlElement *elemInfo);
            virtual void setSystem(Subsystem *sys);
            Subsystem* getSystem();

            virtual QWidget* openDialogStage();

            size_t pinIndex(Pin *pin);

            /**
             * Pin name format: ClassSimpleName.index
             * @param pin
             * @return
             */
            Pin* getPin(size_t index);

            const vector<GraphFlowData>& getFlows();

            virtual XmlParser::XmlElement* to_xml();

            /**
             * path from root to this element
             */
            string getPath();

            void setRotation(int value);

            void setSelected(bool value);
            bool isSelected();

            int getRotation();

            double getX();

            double getY();

            string const& getImgPath();

            void moveTo(QPointF p);

            void moveTo(double x, double y);

            void moveBy(double x, double y);

            void rotate();

            void toFront();

            virtual void initView();
            ElementWidget* getView();

            virtual QWidget* getLibView();

            /**
             * @return the name
             */
            string getName();

            Element* clone();

            /**
             * @param name the name to set
             */
            virtual void setName(string const& name);

            vector<shared_ptr<Pin>>& _getPins();

            string getDiscreteFunction();
            const vector<MathPack::StringGraph>& getEquations();

            const vector<InitParam>& getInitParams();

            // ODE API
            virtual void discreteStep(double time);
            virtual void continuousStep();
            virtual void discreteInit();
            virtual void discreteOutputsInit(Pin *inheritFrom=nullptr);
            const WorkSpace::WorkSpace& getWorkspace(bool refreshValues=true);
            bool isDiscrete();
            bool isPhysical();

        friend class View;
    };
}

#include "Pin.h"
#include "./Subsystem/Subsystem.h"
#include "ElementFactory.h"

#endif
