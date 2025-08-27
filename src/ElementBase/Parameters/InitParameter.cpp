#include "Parameter.h"
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

namespace ElementBase{
    bool InitParam::getPriority() const {
        return(priority);
    }

    void InitParam::setPriority(bool val){
        priority = val;
        tmp_priority = val;
    }

    void InitParam::update(){
        Parameter::update();
        priority = tmp_priority;
    }

    vector<QWidget*> InitParam::getView(){
        vector<QWidget*> ret = Parameter::getView();

        // priority
        QComboBox *box = new QComboBox();
        box->addItems({"Low", "High"});
        box->setCurrentIndex(priority);
        tmp_priority = priority;
        QWidget::connect(box, &QComboBox::currentIndexChanged, [this, box](){tmp_priority = box->currentIndex();});
        ret.insert(ret.begin()+1, box);

        return ret;
    }

    XmlParser::XmlElement* InitParam::to_xml(){
        XmlParser::XmlElement* ret = Parameter::to_xml();
        ret->attributes[ATTR_INIT_PRIOR] = priority ? "1":"0";

        return ret;
    }

    void InitParam::loadState(XmlParser::XmlElement *data){
        Parameter::loadState(data);
        
        if (data->attributes.contains(ATTR_INIT_PRIOR))
            setPriority(XmlParser::stob(data->attributes[ATTR_INIT_PRIOR]));
    }
}
