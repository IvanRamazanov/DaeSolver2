#ifndef ELAMBASE_PARAMS_H
#define ELAMBASE_PARAMS_H


#include <QString>
#include <QWidget>
#include <string>
#include "../ElemBaseDecl.h"
#include "../../MathPack/Parser.h"
#include "../../Connections/Property.h"

namespace ElementBase{
    class Parameter{
        protected:
            QString name;
            QString title;
            QString tmpTextValue;

            pair<int, int> sizeSetting = {-1, 0}, // -1 means inherit from value
                            actualSize; 
            vector<double> value; // TODO what about matrix? variant?
            function<void ()> onChange;


        private:
            void validateDimensions(size_t newSize);

            void setValue(double val);

            void setValue(vector<double> const& val);

        protected:
            virtual string getStringValue();
            
        public:
            Parameter(string const& name, double value, string const& title=string());
            Parameter(string const& name, vector<double> const& value, string const& title=string(), int size=-1);

            virtual ParamType holds();
            virtual XmlParser::XmlElement* to_xml();
            virtual void loadState(XmlParser::XmlElement *data);
            virtual vector<QWidget*> getView();

            virtual void setValue(string const& val);
            
            /**
             * Actual size
             */
            size_t getSize();

            string getName();
            void setName(string const& name);

            const QString& getTitle();
            
            /**
             * fetch data from UI and parse it
             */
            virtual void update();

            void requestFocus();

            /**
             * Value at t=0.
             * @return
             */
            double getScalarValue() const;

            vector<double> getVectorValue() const;

            void setOnChange(function<void ()> onChange);
    };

    class InitParam : public Parameter{
        private:
            bool priority = false,
                tmp_priority = false;

        public:
            /**
             *
             * @param name
             * @param initVal
             */
            using Parameter::Parameter;

            bool getPriority() const;
            void setPriority(bool val);

            XmlParser::XmlElement* to_xml() override;
            void loadState(XmlParser::XmlElement *data) override;
            void update() override;
            vector<QWidget*> getView() override;
    };

    class ComboBoxParam : public Parameter {
        vector<string> choices, data;
        QString selection;

        protected:
            string getStringValue() override;
        
        public:
            ComboBoxParam(string const& name, string const& title, string const& defaultSelection, vector<string> const& choices, vector<string> const& data);

            vector<QWidget*> getView() override;

            ParamType holds() override;

            void setValue(string const& val) override;
            
    };
}

#endif
