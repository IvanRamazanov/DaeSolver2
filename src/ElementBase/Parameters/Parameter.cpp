#include "Parameter.h"
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <sstream>

namespace ElementBase{
    QRegularExpression namePrefix("^[0-9]");

    Parameter::Parameter(string const& name, double value, string const& title)
    {
        setName(name);
        this->title = title.empty() ? this->name : QString::fromStdString(title);

        this->value.resize(1);
        sizeSetting = {1, 0};
        setValue(value);
    }

    Parameter::Parameter(string const& name, vector<double> const& value, string const& title, int size)
    {
        setName(name);
        this->title = title.empty() ? this->name : QString::fromStdString(title);

        sizeSetting = {size, 0};
        setValue(value);
    }

    void Parameter::setName(string const& name){
        // name can't have whitespaces
        if (name.find(' ')!=string::npos){
            ostringstream oss;
            oss << "Param: " << name << endl;
            oss << "Parameter name should follow variable name rules: no whitespaces!";
            throw invalid_argument(oss.str());
        }
        this->name = QString::fromStdString(name);
        auto match = namePrefix.match(this->name);
        if (match.hasMatch())
            throw invalid_argument("Parameter name should follow variable name rules: can't start with digits!");
    }

    string Parameter::getName() {
        return name.toStdString();
    }

    const QString& Parameter::getTitle(){
        return title;
    }

    void Parameter::update(){
        try{
            setValue(tmpTextValue.toStdString());
        }catch(exception &e){
            // failed, restore text value
            tmpTextValue = QString::fromStdString(getStringValue());
        }
    }

    void Parameter::setValue(string const& val){
        if (sizeSetting.first == -1 && sizeSetting.second==-1){
            // accept everything
            //value = XmlParser::parseMat(val);
            actualSize.first = value.size();
            //actualSize.second = value[0].size();
        }else if(sizeSetting.second == 0){
            actualSize.second = 0;
            // val is vector or scalar
            if(sizeSetting.first == 1){
                // strictly a scalar
                value.resize(1);
                value[0] = stod(val);
                actualSize.first = 1;
            }else{
                auto tmpVec = XmlParser::parseRow(val);
                if(sizeSetting.first != -1){
                    validateDimensions(tmpVec.size());
                }

                value = tmpVec;
                actualSize.first = value.size();
            }
        }else{
            // TODO fixed size matrix
        }

        tmpTextValue = QString::fromStdString(val);
    }

    size_t Parameter::getSize(){
        return actualSize.first;
    }

    XmlParser::XmlElement* Parameter::to_xml(){
        XmlParser::XmlElement* ret = new XmlParser::XmlElement(PARAM_TAG);

        ret->attributes[ATTR_PARAM_NAME] = getName();
        ret->attributes[ATTR_PARAM_TYPE] = to_string(holds());
        ret->text = tmpTextValue.toStdString();

        return ret;
    }

    void Parameter::loadState(XmlParser::XmlElement *data){
        if(!data->text.empty())
            setValue(data->text);

        // old version (enforces type match)
        // if (data->attributes.contains(ATTR_PARAM_TYPE))
        //     if (stoi(data->attributes[ATTR_PARAM_TYPE]) == holds()){
        //         tmpTextValue = QString::fromStdString(data->text);
        //         update();
        //     }
    }

    double Parameter::getScalarValue() const {
        return value[0];
    }

    vector<double> Parameter::getVectorValue() const {
        return value;
    }

    void Parameter::setValue(double val){
        value[0] = val;
        actualSize = {1, 0};

        tmpTextValue = QString("%1").arg(val, 0, 'f', -1);
    }

    void Parameter::setValue(vector<double> const& val){
        validateDimensions(val.size());

        value = val;
        actualSize = {val.size(), 0};

        tmpTextValue = QString::fromStdString(XmlParser::vec_to_s(value));
    }

    void Parameter::setOnChange(function<void ()> onChange){
        this->onChange = onChange;
    }

    vector<QWidget*> Parameter::getView(){
        vector<QWidget*> ret;

        // value edit
        QLineEdit *text = new QLineEdit(tmpTextValue);
        QWidget::connect(text, &QLineEdit::editingFinished, [this, text](){tmpTextValue = text->text();});

        ret.push_back(new QLabel(title));
        ret.push_back(text);

        return ret;
    }

    void Parameter::requestFocus(){
        //text->requestFocus();
    }

    ParamType Parameter::holds(){
        if (actualSize.second != 0){
            return Matrix;
        }else if (actualSize.first == 1){
            return Scalar;
        }else{
            return Vector;
        }
    }

    string Parameter::getStringValue(){
        switch(holds()){
            case Scalar:
                return to_string(value[0]);
            case Vector:
                return XmlParser::vec_to_s(value);
            default:
                //return XmlParser::mat_to_s(value); TODO implement(?)
                return "69";
        }
        
    }

    void Parameter::validateDimensions(size_t newSize){
        if(sizeSetting.first == -1)
            // just accept
            return;
        else if(sizeSetting.first != newSize)
            throw runtime_error("Parameter dimensions don't match! Expect:"+to_string(sizeSetting.first)+" got:"+to_string(newSize));
        
        // TOOD implement matices(?)
    }
}