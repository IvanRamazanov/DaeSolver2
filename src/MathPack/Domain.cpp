#include "Domain.h"

using namespace std;

namespace Domains{
    /**
     * class Domain
     */

    Domain::Domain(XmlParser::XmlElement *data):
        typeName(data->name),
        typeId(data->attributes.contains("type") ? data->attributes["type"] : typeName),
        typeTitle(data->attributes.contains("name") ? data->attributes["name"] : typeName),
        varSize(data->attributes.contains("size") ? stoi(data->attributes["size"]) : 1),
        wireColour(data->attributes.contains("color") ? QColor::fromString(data->attributes["color"]) : QColor("#000000")),
        directional(data->attributes.contains("discrete") ? XmlParser::stob(data->attributes["discrete"]) : false)
    {
        for(auto& dom:domains){
            if (dom->typeName == typeName){
                runtime_error("Domain already exists: " + typeName);
            }
        }

        // pin shape
        QString tmpPath;
        if (auto d = data->find("pin"))
            tmpPath = QString::fromStdString(d->text);
        if (!tmpPath.isEmpty() && QFile(tmpPath).exists()){
            pinShape = new QSvgRenderer(tmpPath);
        }else{
            // Electric by default
            pinShape = new QSvgRenderer(QString::fromStdString(":/src/data/Pins/ElectricPin.svg"));
        }

        // marker shape
        tmpPath.clear();
        if (auto d = data->find("marker"))
            tmpPath = QString::fromStdString(d->text);
        if (!tmpPath.isEmpty() && QFile(tmpPath).exists()){
            markerShape = new QSvgRenderer(tmpPath);
        }else{
            // Electric by default
            markerShape = pinShape;
        }

        domains.push_back(this);
    }

    Domain::~Domain(){
        // Note: normally, it's ok to just delete it. But in general make sure all pins that use it are deleted first
        delete pinShape;
        if (pinShape != markerShape){
            pinShape = nullptr;
            delete markerShape;
        }
    }

    unique_ptr<Domain> Domain::_fromId(string const& domId){
        QFile f(":/src/data/Connections/Domains.xml");
        if(!f.open(QIODevice::ReadOnly))
            throw runtime_error("Default file Domains.xml doesn't exist!");
        string s = QString(f.readAll()).toStdString();
        auto et = XmlParser::ElementTree::fromString(s);
        auto data = et->getRoot()->find(domId);
        if (data == nullptr)
            throw invalid_argument("Default Domain doesn't exist: " + domId);

        return make_unique<Domain>(data);
    }

    Domain* Domain::getDomain(string const& name){
        for (auto& dom:domains){
            if (dom->typeName == name){
                return dom;
            }
        }
        return nullptr;
    }

    Connector Domain::getConnector(string const& pinName){
        Connector ret;
        if(directional){
            ret.varNames.push_back(pinName);
            ret.varTypes.push_back(WorkSpace::WsDataType::Discrete);
        }else{
            for (int i=0; i<varSize; i++){
                string sizeSuffix = varSize>1 ? std::format("[{}]", i+1) : "",
                    potName = pinName+".p"+sizeSuffix,
                    flowName = pinName+".f"+sizeSuffix;
                ret.varNames.push_back(potName);
                ret.varTypes.push_back(WorkSpace::WsDataType::Potential);
                ret.varNames.push_back(flowName);
                ret.varTypes.push_back(WorkSpace::WsDataType::Flow);
            }
        }
        return ret;
    }

    QSvgRenderer* Domain::getPinRenderer(){
        return pinShape;
    }

    QSvgRenderer* Domain::getMrkRenderer(){
        return markerShape;
    }

    const vector<Domain*>& Domain::getDomains(){
        return domains;
    }

    ConnDirection s2dir(string const& str){
        if (str == "in"){
            return ConnDirection::Input;
        }else if (str == "out"){
            return ConnDirection::Output;
        }else if (str == "uni"){
            return ConnDirection::Uni;
        }else{
            throw invalid_argument("Invalid direction value: " + str);
        }
    }

    string dir2s(ConnDirection const& dir){
        switch (dir){
            case ConnDirection::Input:
                return "in";
            case ConnDirection::Output:
                return "out";
            case ConnDirection::Uni:
                return "uni";
        }
    }

    unique_ptr<Domain> ELECTRIC = Domain::_fromId("elec");
    unique_ptr<Domain> ELECTRIC_3PH = Domain::_fromId("elec_3ph");
    unique_ptr<Domain> MATH = Domain::_fromId("math");
    unique_ptr<Domain> MECH_ROTATIONAL = Domain::_fromId("rot");
}
