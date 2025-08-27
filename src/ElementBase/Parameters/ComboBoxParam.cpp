#include "Parameter.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>

namespace ElementBase{
    ComboBoxParam::ComboBoxParam(string const& name, string const& title, string const& defaultSelection, vector<string> const& choices, vector<string> const& data) : 
        Parameter(name, 0, title),
        choices(choices),
        data(data)
    {
        if (choices.size() != data.size()){
            throw runtime_error("choices and data input arguments must have same size");
        }
        if (choices.size() < 1){
            throw runtime_error("Must have atleast one choice");
        }

        bool setVal = false;
        for(auto i = data.size(); i-->0;){
            if (data[i] == defaultSelection){
                selection = QString::fromStdString(choices[i]);
                setVal = true;
                break;
            }
        }
        if(!setVal)
            selection = QString::fromStdString(choices[0]);
    }

    vector<QWidget*> ComboBoxParam::getView() {
        vector<QWidget*> ret;

        // value edit
        QComboBox *text = new QComboBox();
        text->setInsertPolicy(QComboBox::NoInsert);
        
        for (size_t i = 0; i<data.size(); i++){
            text->insertItem(i, QString::fromStdString(choices[i]), QVariant(QString::fromStdString(data[i])));
        }
        tmpTextValue = selection;
        text->setCurrentText(tmpTextValue);

        QWidget::connect(text, &QComboBox::currentTextChanged, [&](){tmpTextValue = text->currentText();});

        ret.push_back(new QLabel(name));
        ret.push_back(text);

        return ret;
    }

    string ComboBoxParam::getStringValue(){
        return selection.toStdString();
    }

    ParamType ComboBoxParam::holds(){
        return ParamType::ComboBox;
    }

    void ComboBoxParam::setValue(string const& value){
        for (auto& d:data){
            if (d == value){
                selection = QString::fromStdString(d);
                break;
            }
        }
    }
}
