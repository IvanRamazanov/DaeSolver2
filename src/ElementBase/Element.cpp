/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "Element.h"
#include <sstream>

using namespace std;
using Domain = Domains::Domain;
using Direction = Domains::ConnDirection;

/**
 *
 * @author Ivan
 */
namespace ElementBase{
    // DnD
    const QString CUSTOM_FORMAT = QString("application/dae-solver.rim");
    auto constexpr ELEM_EQUATIONS_TAG = "Equations";
    auto constexpr ELEM_EQU_ENTRY_TAG = "equ";
    auto constexpr ELEM_BRANCHES_TAG = "InternalFlow";
    auto constexpr ELEM_BRANCH_ENTRY_TAG = "sum";
    auto constexpr ELEM_LOCAL_VARS_TAG = "Workspace";
    auto constexpr ELEM_VAR_TAG = "var";
    auto constexpr ELEM_CONST_TAG = "const";
    auto constexpr VAR_INIT_PARAM = "init";

    void __addPinToWs(shared_ptr<Pin> const& pin, WorkSpace::WorkSpace & ws){
        auto& conn = pin->connector;
        if (pin->getDirection() == Direction::Uni){
            for (size_t i=0; i<conn.varNames.size(); i++){
                ws.add(conn.varNames[i], 0, false, conn.varTypes[i]);
                
                // create diffs by default
                ws.addDiff(conn.varNames[i], conn.varTypes[i]);
            }
        }else{
            for (size_t i=0; i<conn.varNames.size(); i++){
                ws.addVector(conn.varNames[i], vector<double>(pin->getDataSize(), 0));
                if (pin->is_cached)
                    ws.addVector(conn.varNames[i]+PIN_CACHE_VAR_SUFFIX, vector<double>(pin->getDataSize(), 0));
            }
        }
    }

    /**
     * Element class
     */

    Element::Element(){
    }

    Element::Element(filesystem::path path){
        shared_ptr<XmlParser::XmlElement> spec;
        if (*path.begin() == "Custom"){
            // TODO custom components; open file outside of resources
            // spec = ???
        }else{
            id = path.string();
            // general case; spec is in resources
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text)){
                throw ios_base::failure("File not found: " + path.string());
            }

            QTextStream in(&f);
            auto text = in.readAll().toStdString();
            auto etree = XmlParser::ElementTree::fromString(text);
            spec = etree->getRoot();
            delete etree;
        }

        // view
        if (spec->attributes.contains(ATTR_ELEM_VIEW)){
            imgName = spec->attributes[ATTR_ELEM_VIEW];
            imgPath = path.parent_path().string() + "/" + imgName; //for creation!
        }else{
            imgPath = ":/src/data/Elements/Subsystem.png"; // default view
        }

        // unique name
        if (spec->attributes.contains(ATTR_ELEM_NAME)){
            name = spec->attributes[ATTR_ELEM_NAME];
        }else{
            name = path.stem().string(); // name == id
        }

        // description
        if (spec->attributes.contains(ATTR_ELEM_DESCR)){
            description = spec->attributes[ATTR_ELEM_DESCR];
        }

        // params
        auto prms = spec->find(ELEM_PARAMS_TAG);
        if(prms){
            for (auto& p:prms->findAll(PARAM_TAG)){
                addParameter(p);
            }
        }

        // pins
        auto conns = spec->find(CONN_TAG);
        if (conns){
            auto pin_specs = conns->findAll(PIN_TAG);
            for (auto& pin:pin_specs){
                // add to list
                pins.push_back(make_unique<Pin>(this, pin));
                // add to ws
                __addPinToWs(pins.back(), workspace);

                // inits
                auto initList = pin->findAll(VAR_INIT_PARAM);
                for(auto& ip:initList){
                    if (ip->attributes.contains(ELEM_VAR_TAG)){
                        bool found=false;
                        for(auto& pv:pins.back()->connector.varNames){
                            if (pv == ip->attributes[ELEM_VAR_TAG]){
                                // found match for init param
                                addInitValue(ip,
                                    dynamic_cast<WorkSpace::DifferentialVar*>(workspace.get(WorkSpace::DIFF_PTRFIX +pv))
                                );

                                found = true;
                                break;
                            }
                        }
                        if (!found){
                            cerr << "Domain of pin:"<<pins.back()->getName()<<" doesn't have variable: "<<ip->attributes[ELEM_VAR_TAG]<<endl;
                        }
                    }else{
                        cerr << "Init param must have associated var specified! Element: "<<name<<endl;
                    }
                }
            }
        }

        // equations
        conns = spec->find(ELEM_EQUATIONS_TAG);
        if (conns){
            auto equs = conns->findAll(ELEM_EQU_ENTRY_TAG);
            for (auto& eq:equs){
                string elemFunc = eq->text;
                elemFunc.erase(remove(elemFunc.begin(), elemFunc.end(), ' '), elemFunc.end()); // not necessary, but to speed up?
                int eqSignIdx = -1;
                // find equality sign
                for(size_t k=0; k<elemFunc.length(); k++){
                    if(elemFunc[k] == '='){
                        eqSignIdx = k;
                        break;
                    }
                }
                if (eqSignIdx == -1)
                    throw runtime_error("Equation must have an equality sign: " + eq->text);
                // add to list
                MathPack::StringGraph equation(elemFunc.substr(0, eqSignIdx));
                auto right = make_shared<MathPack::StringGraph>(elemFunc.substr(eqSignIdx+1));
                equation.sub(right); // to make it 0 = ...

                equations.emplace_back(equation);
            }

            // local eq vars
            equs = conns->findAll("var");
            for (auto& var:equs){
                if (!var->attributes.contains("name"))
                    throw runtime_error("Equation var must have a name: " + var->text);

                if (var->text.empty())
                    continue;

                string varName = var->attributes["name"];
                auto right = make_shared<MathPack::StringGraph>(var->text);

                // replace local var in equations
                for(auto& eq:equations)
                    eq.replaceVariable(varName, right);
            }
        }

        // flows
        conns = spec->find(ELEM_BRANCHES_TAG);
        if (conns){
            auto equs = conns->findAll(ELEM_BRANCH_ENTRY_TAG);
            for (auto& eq:equs){
                // in the list are Pins or individual Pin connector vars:
                bool containsVars = eq->attributes.contains("by_var") ? XmlParser::stob(eq->attributes["by_var"]) : false;
                // add to list
                auto pList = XmlParser::split(eq->text, ";");
                GraphFlowData tmpEq;
                // find index of a pin
                for (auto& p:pList){
                    if(p == "*"){
                        if (containsVars)
                            throw runtime_error("Invalid flow setting: * (reference) doesn't support by-var assignment");

                        tmpEq.setReference();
                        break; // ref (*) could be only the last operand
                    }else{
                        for (size_t i=0; i<pins.size(); i++){
                            if(!containsVars){
                                // look for matching pin name
                                if (pins[i]->getName() == p){
                                    // match found
                                    tmpEq.addPin(pins[i].get());
                                }
                            }else{
                                // look for matching pin conn var name
                                for (size_t j=0; j<pins[i]->connector.varNames.size(); j++){
                                    auto &vn = pins[i]->connector.varNames[j];
                                    if(vn == p){
                                        // match found
                                        tmpEq.addVar(vn, pins[i].get());
                                    }
                                }
                            }
                        }
                    }
                }
                if(!tmpEq.isEmpty())
                    flows.push_back(tmpEq);
            }
        }

        // local vars
        conns = spec->find(ELEM_LOCAL_VARS_TAG);
        if (conns){
            // vars
            auto local_vars = conns->findAll(ELEM_VAR_TAG);
            for (auto& v:local_vars){
                if (!v->attributes.contains("name"))
                    throw runtime_error("Local var 'name' parameter is requred!");
                string var_name = v->attributes["name"];
                auto type = WorkSpace::WsDataType::Discrete;
                if (v->attributes.contains("type")){
                    int vt = stoi(v->attributes["type"]);
                    type = WorkSpace::WsDataType(vt);
                }

                workspace.add(var_name, 0, false, type);
                auto newDiff = workspace.addDiff(var_name, type);

                // inits
                auto initVal = v->find(VAR_INIT_PARAM);
                if(initVal){
                    addInitValue(initVal, newDiff);
                }
            }

            // constants
            local_vars = conns->findAll(ELEM_CONST_TAG);
            for (auto& v:local_vars){
                if (!v->attributes.contains("name"))
                    throw runtime_error("Local var 'name' parameter is requred!");
                string var_name = v->attributes["name"];

                double varVal = 0;
                if (!v->text.empty())
                    varVal = stod(v->text); // Note: can fail!

                workspace.add(var_name, varVal, true, WorkSpace::WsDataType::Discrete);
            }
        }
    }

    Element::~Element(){
        for (auto& pin:pins){
            pin->unPlug();
        }
        delete _view;
    }

    QWidget* Element::openDialogStage() {
        QWidget *subWind = new QWidget();
        subWind->setAttribute(Qt::WA_DeleteOnClose);
        subWind->setWindowTitle(QString::fromStdString("Parameters: " + getName()));
        QVBoxLayout *root = new QVBoxLayout();

        QLabel *header = new QLabel();
        header->setText(QString::fromStdString(getName()+"\n\n"));
        header->setFont(QFont("Times New Roman", 12, QFont::Bold));
        root->addWidget(header);

        // description
        root->addWidget(new QLabel(QString::fromStdString(getDescription())));

        // settings
        QTabWidget *pane = new QTabWidget();
        //pane->setTabClosingPolicy(TabPane.TabClosingPolicy.UNAVAILABLE);

        // params
        QVBoxLayout *top = new QVBoxLayout();
        for(auto& p:parameters){
            auto pn = p->getView();
            QWidget* root = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout();
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(4);
            for (auto& sw:pn)
                layout->addWidget(sw);
            root->setLayout(layout);
            root->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
            
            top->addWidget(root);
        }
        QWidget *params = new QWidget();
        //QScrollArea *params_view = new QScrollArea();
        //params_view->setWidget(params);
        params->setLayout(top);
        //params_view->setVbarPolicy(ScrollPane.ScrollBarPolicy.AS_NEEDED);
        pane->addTab(params, "Element parameters");
        
        // initial state params
        if(!stateInitValues.empty()){
            QWidget *inits = new QWidget();
            QGridLayout *ttop = new QGridLayout();
            // header
            ttop->addWidget(new QLabel("Name"), 0, 0);
            ttop->addWidget(new QLabel("Priority"), 0, 1);
            ttop->addWidget(new QLabel("Value"), 0, 2);
            // content
            for(size_t k=0; k<stateInitValues.size(); k++){
                InitParam& p=stateInitValues[k];
                auto pn = p.getView();
                for(size_t i=pn.size(); i-->0;){
                    ttop->addWidget(pn[i], k+1, i);
                }
            }
            //ttop.getColumnConstraints().add(new ColumnConstraints(Control.USE_COMPUTED_SIZE));
            //ttop.getColumnConstraints().add(new ColumnConstraints(Control.USE_COMPUTED_SIZE));
            //ttop.getColumnConstraints().add(new ColumnConstraints(Control.USE_COMPUTED_SIZE));
            ttop->setHorizontalSpacing(2);
            inits->setLayout(ttop);
            pane->addTab(inits, "Initial conditions");
        }

        root->addWidget(pane);

        //---buttns
        QHBoxLayout *bot = new QHBoxLayout();
        QPushButton *btn = new QPushButton("Apply");
        QWidget::connect(btn, &QPushButton::clicked, [this, subWind](){
            // to force any QLineEdit to update
            if(auto w = subWind->focusWidget())
                w->clearFocus();

            for(auto& data:parameters){
                data->update();
            }
            for(auto& data:stateInitValues){
                data.update();
            }
            subWind->close();
        });
        btn->setShortcut(QKeySequence(Qt::Key_Return));
        bot->addWidget(btn);
        btn = new QPushButton("Cancel");
        QWidget::connect(btn, &QPushButton::clicked, [subWind](){
            subWind->close();
        });
        bot->addWidget(btn);
        bot->setAlignment(Qt::AlignRight);
        QWidget *btnGroup = new QWidget();
        btnGroup->setLayout(bot);
        root->addWidget(btnGroup);

        subWind->setLayout(root);
        if(parameters.size()>0){
            parameters[0]->requestFocus();
        }

        return subWind;
    }

    void Element::loadState(XmlParser::XmlElement *elemInfo){
        // name
        if(elemInfo->attributes.count(ATTR_ELEM_NAME))
            setName(elemInfo->attributes[ATTR_ELEM_NAME]);
        else throw runtime_error("Element name attribute is required!");

        auto lay = elemInfo->find(ELEM_LAYOUT_TAG);
        if(lay){
            auto layout = XmlParser::split(lay->text, ",");
            double  x = stod(layout[0]),
                    y = stod(layout[1]),
                    rotate = stod(layout[2]);
            this->x = x;
            this->y = y;
            setRotation(rotate);
        }

        //parameter cycle
        auto param_root = elemInfo->find(ELEM_PARAMS_TAG);
        if (param_root){
            auto params = param_root->findAll(PARAM_TAG);
            for (auto& pd:params){
                if (pd->attributes.contains(ATTR_PARAM_NAME))
                    for (auto& p:parameters){
                        if (p->getName() == pd->attributes[ATTR_PARAM_NAME]){
                            p->loadState(pd);
                            break;
                        }
                    }
            }
        }

        // init state cycle
        param_root = elemInfo->find(ELEM_INITS_TAG);
        if (param_root){
            auto params = param_root->findAll(PARAM_TAG);
            for (auto& p:params){
                // validate
                if (!p->attributes.contains(ATTR_PARAM_NAME)) throw runtime_error("Parameter name is requred!");

                for (auto& ip:stateInitValues){
                    if (ip.getName() == p->attributes[ATTR_PARAM_NAME]){
                        ip.loadState(p);
                        break;
                    }
                }
            }
        }
    }

    void Element::initFlags(){
        // if at least one pin is dynamic (non directional)
        physicalFlag = false;
        for (auto& pin:pins)
            if (!pin->getDomain()->directional){
                physicalFlag = true;
                break;
            }

        discreteFlag = !discreteFunction.empty();
        // or if has at least one discrete pin
        for (auto& p:pins){
            if(p->getDirection() != Direction::Uni){
                discreteFlag = true;
                break;
            }
        }
    }

    Element* Element::clone(){
        auto data = to_xml();
        auto ret = makeElement(data);
        delete data;
        return ret;
    }

    // void Element::setHeight(double val){
    //     //elemBody->setHeight(val); // transform?
    // }

    void Element::setSystem(Subsystem *sys){
        if (system != nullptr){
            // already in some system.
            // remove itself from system view
            system->_getScene()->removeItem(_view);
        }

        system = sys;
        setName(system->getUniqueName(name));

        system->_getScene()->addItem(_view);
        // add external pins
        for (auto& p:pins){
            if(p->getOwner()==this && p->isExternal())
                sys->addPinView(p, p->getDirection()!=Domains::Output);
        }
    }

    string Element::getDescription(){
        return description;
    }

    Subsystem* Element::getSystem(){
        return system;
    }

    Pin* Element::_addPin(string const& name, Domain *dom, bool isExternal, Direction direction){
        try{
            auto np = make_shared<Pin>(name, this, QPointF(), dom, isExternal, direction);
            pins.push_back(np);

            // append workspace
            __addPinToWs(pins.back(), workspace);

            return pins.back().get();
        }catch (exception &e){
            cerr << e.what() << endl;
            return nullptr;
        }
    }

    Pin* Element::addPin(string const& name, QPointF const& pinPos, Domain *dom, bool isExternal, Direction direction){
        Pin *ret = _addPin(name, dom, isExternal, direction);
        if(ret){
            ret->moveCenterTo(pinPos);
        }

        return ret;
    }

    Pin* Element::addPin(string const& name, Direction direction){
        Pin *ret = _addPin(name, Domains::MATH.get(), false, direction);
        if(ret){
            alignPin(ret, direction==Domains::Input);
        }

        return ret;
    }

    bool Element::isDiscrete(){
        return discreteFlag;
    }

    bool Element::isPhysical(){
        return physicalFlag;
    }

    string Element::getDiscreteFunction(){
        return discreteFunction;
    }

    const vector<MathPack::StringGraph>& Element::getEquations(){
        return equations;
    }

    void Element::setSelected(bool value){
        (void)value;
        // if (_view){
        //     if (value)
        //         _view->elemBody->setGraphicsEffect(_view->selectedEffect);
        //     else
        //         _view->elemBody->setGraphicsEffect(nullptr);
        // }
    }

    bool Element::isSelected(){
        return _view->elemBody->view->isSelected();
    }

    void Element::moveBy(double x, double y){
        this->x += x;
        this->y += y;
        
        _view->moveBy(x, y);
        
        for (auto& p:pins)
            p->updatePosition();
    }

    void Element::moveTo(double x, double y){
        this->x = x;
        this->y = y;
        
        _view->setPos(x, y);
        
        for (auto& p:pins)
            p->updatePosition();
    }

    void Element::moveTo(QPointF p){
        this->x = p.x();
        this->y = p.y();
        
        _view->setPos(p);
        
        for (auto& p:pins)
            p->updatePosition();
    }

    double Element::getX(){
        return x;
    }

    double Element::getY(){
        return y;
    }

    const vector<GraphFlowData>& Element::getFlows(){
        return flows;
    }

    size_t Element::pinIndex(Pin *pin){
        for (size_t i=pins.size(); i-->0;)
            if (pins[i].get() == pin)
                return i;
        return -1;
    }

    /**
     * Pin name format: ClassSimpleName.index
     * @param pin
     * @return
     */
    Pin* Element::getPin(size_t index){
        // auto pin_dom = Domains::Domain::getDomain(pin_type);
        // if (!pin_dom) throw runtime_error("Invalid pin domain name: " + pin_type);

        for (auto& p:pins){
            // if (p->getDomain() == pin_dom){
                if(index == 0)
                    return p.get();
                else
                    index--;
            // }
        }
        return nullptr;
    }

    XmlParser::XmlElement* Element::to_xml(){
        XmlParser::XmlElement *ret= new XmlParser::XmlElement(ELEM_TAG);

        ret->attributes[ATTR_ID] = this->id;
        ret->attributes[ATTR_ELEM_NAME] = name;
        //ret->attributes[ATTR_ELEM_DESCR] = description;

        //if (!imgName.empty())
        //    ret->attributes[ATTR_ELEM_VIEW] = imgName;

        XmlParser::XmlElement *layout = new XmlParser::XmlElement(ELEM_LAYOUT_TAG);
        layout->text = format(ELEM_LAYOUT_FORMAT, x, y, rot);
        ret->append(layout);

        // TODO: pins

        XmlParser::XmlElement *params = new XmlParser::XmlElement(ELEM_PARAMS_TAG);
        for(auto& param:parameters){
            params->append(param->to_xml());
        }
        ret->append(params);

        params = new XmlParser::XmlElement(ELEM_INITS_TAG);
        for(auto& param:stateInitValues){
            params->append(param.to_xml());
        }
        ret->append(params);

        return ret;
    }

    void Element::addParameter(XmlParser::XmlElement *info){
        // validate name
        if (!info->attributes.contains(ATTR_PARAM_NAME)){
            cerr << "Parameter name is required!" << endl;
            return;
        }
        string name = info->attributes[ATTR_PARAM_NAME];

        // validate title
        if (!info->attributes.contains(ATTR_PARAM_TITLE)){
            cerr << "Parameter title is required! " << name << endl;
            return;
        }
        string title = info->attributes[ATTR_PARAM_TITLE];

        if (!info->attributes.contains(ATTR_PARAM_TYPE)){
            cerr << "Param type is required!" << name << endl;
            return;
        } 
        ParamType type = ParamType(stoi(info->attributes[ATTR_PARAM_TYPE]));

        addParameter(name, title, info->text, type);
    }

    void Element::addParameter(string const& name, string const& title, string const& value, ParamType pType){
        // validate type
        if (pType == ParamType::Matrix){
            //parameters.push_back(make_unique<MatrixParameter>(name, title, XmlParser::parseMat(value)));
            // TODO matrix WS params are not supported yet!
        }else if (pType == ParamType::Vector){
            vector<double> pValue = XmlParser::parseRow(value);
            parameters.push_back(make_unique<Parameter>(name, pValue, title));
            workspace.addVector(name, pValue);
        }else if (pType == ParamType::Scalar){
            double pValue = stod(value);
            parameters.push_back(make_unique<Parameter>(name, pValue, title));
            // add to workspace
            workspace.add(name, pValue, true);
        }else cerr << "Unsupported type: " << pType << endl;
    }

    Parameter* Element::addScalarParameter(string const& name, string const& title, double value){
        parameters.emplace_back(make_unique<Parameter>(name, value, title));
        workspace.add(name, value);

        return parameters.back().get();
    }

    Parameter* Element::addVectorParameter(string const& name, string const& title, vector<double> const& value){
        parameters.emplace_back(make_unique<Parameter>(name, value, title));
        workspace.addVector(name, value);

        return parameters.back().get();
    }

    void Element::addInitValue(XmlParser::XmlElement *info, WorkSpace::DifferentialVar *var){
        if(var == nullptr){
            cerr << "Can't find diff for init param:"<<var<< endl << "Element: "<<name<<endl;
            return;
        }

        string name, title;
        if (!info->attributes.contains(ATTR_PARAM_NAME)){
            cerr << "Parameter name is required!" << endl;
            name = var->baseVar->getName();
        }else{
            name = info->attributes[ATTR_PARAM_NAME];
        }

        if (!info->attributes.contains(ATTR_PARAM_TITLE)){
            title = name;
        }else{
            title = info->attributes[ATTR_PARAM_TITLE];
        }

        // validate type
        double val;
        if (info->attributes.contains(ATTR_PARAM_TYPE)){
            int type = stoi(info->attributes[ATTR_PARAM_TYPE]);
            if (type == ParamType::Scalar){
                val = stod(info->text);
            }else{
                throw runtime_error("Unsupported type: " + to_string(type) + " " + name);
            } 
        }else{
            try{
                val = stod(info->text);
            }catch (exception &e){
                (void)e;
                val = 0;
            }
        }

        stateInitValues.emplace_back(InitParam(name, val, title));

        // assign to var
        var->setInitVal(&stateInitValues.back());
    }

    void Element::addInitValue(string const& name, string const& title, vector<double> const& value, WorkSpace::DifferentialVar *var){
        stateInitValues.emplace_back(InitParam(name, value, title));

        // assign to var
        var->setInitVal(&stateInitValues.back());
    }

    const vector<InitParam>& Element::getInitParams(){
        return stateInitValues;
    }

    void Element::rotate(){
        rot = (rot+90)%360;
        _view->setRotation(rot);
        for (auto& p:pins) p->updatePosition();
    }

    void Element::setRotation(int value){
        rot = value;
        _view->setRotation(value);
        for (auto& p:pins) p->updatePosition();
    }

    int Element::getRotation(){
        return rot;
    }

    string Element::getPath(){
        if (system){
            auto sysPath = system->getPath();
            if (sysPath.empty())
                return name;
            return sysPath + ELEM_PATH_DELIM + name;
        }
        // this is root system, return nothing
        return string();
    }

    ElemTypes Element::type(){
        return Elem;
    }

    /**
     * @return the name
     */
    string Element::getName() {
        return name;
    }

    string const& Element::getImgPath(){
        return imgPath;
    }

    void Element::initView(){
        _view = new ElementWidget(this);
 
        // add pins
        for (auto& p:pins){
            if(!p->isExternal())
                _view->addPin(p.get());
        }
    }

    ElementWidget* Element::getView(){
        return _view;
    }

    /**
     * Note: Lib view is owned by lib Widget's layout.
     * (so, it's safe to return a raw pointer)
     */
    QWidget* Element::getLibView(){
        return new ElementWidgetLib(this);
    }

    /**
     * @param name the name to set
     */
    void Element::setName(string const& newName) {
        if (newName == name) return;

        // new name found
        name = newName;

        // update Widget
        _view->nameLabel->setText(QString::fromStdString(name));
    }

    vector<shared_ptr<Pin>>& Element::_getPins(){
        return pins;
    }

    void Element::alignPin(Pin *pin, bool leftSide){
        QPointF pinPos;
        vector<Pin*>& pinVec = leftSide ? leftSidePins : rightSidePins;
        pinPos.setX(leftSide ? -3 : 40); // TODO get current width instead of '40'
        double h = _view->getBodyHeight();

        pinVec.push_back(pin);
        auto pinCnt = pinVec.size();

        // spread ports evenly across body height
        for (size_t i=0; i<pinCnt; i++){
            pinPos.setY((i+1) * h/(pinCnt+1));
            pinVec[i]->moveCenterTo(pinPos);
        }
    }

    void Element::toFront(){
        _view->setZValue(0); // TODO cycle through scene
    }

    void Element::discreteStep(double time){(void)time;}
    void Element::discreteInit(){}
    void Element::continuousStep(){}

    /**
     * set output data dimensions and link connected wire
     * Note: actual data is stored always in workspace! Pin and wire are linked to WS var
     */
    void Element::discreteOutputsInit(Pin *inheritFrom){
        size_t INHERIT_SIZE = -1; // should be global constant?

        for(auto& p:pins){
            if(p->getDirection() == Domains::Output){
                size_t portSize;
                if (p->dataSizeSetting == INHERIT_SIZE){
                    // if nothing to inherit from, set size to 1
                    portSize = inheritFrom ? inheritFrom->dataSize : 1;
                }else{
                    portSize = p->dataSizeSetting;
                }
                initPortSize(p.get(), portSize);
            }
        }
    }

    void Element::initPortSize(Pin *pin, size_t size, Pin* verifyMatch){
        if(verifyMatch){
            // verify that new size matches src pin
            if(verifyMatch->dataSize != size)
                throw runtime_error("Source pin:"+verifyMatch->getName()+" elem:"+getName()+" has incompatible size!\n"
                                    "Has:"+to_string(verifyMatch->dataSize)+"\n"
                                    "Expect:"+to_string(size));
        }

        const auto& wsVar = workspace.get(pin->getName());
        wsVar->init(vector<double>(size, 0.0));

        // link pin
        pin->dataSize = wsVar->getSize();
        pin->data = wsVar->getRef();

        if(pin->isConnected()){
            // link wire
            pin->getConnectedMarker()->getWire()->initDataRef(pin->data, pin->dataSize);
        }
    }

    const WorkSpace::WorkSpace& Element::getWorkspace(bool refreshValues){
        if (refreshValues)
            // update param values first
            for (auto& p:parameters){
                auto wsp = workspace.get(p->getName());
                if (p->holds() == ParamType::Scalar){
                    wsp->setValue(p.get()->getScalarValue());
                }else if(p->holds() == ParamType::Vector){
                    if (wsp->type() == WorkSpace::WsTypes::Vector){
                        wsp->init(p.get()->getVectorValue());
                    }
                    // else throw exception?
                }
                // TODO matrix?
            }

        return workspace;
    }
    


    /* CLASS ELEMENT_MIME */

    bool ElementMime::hasElement(){
        return customData != nullptr;
    }

    void ElementMime::setData(const QString &mimeType, Element* data){
        if (mimeType != CUSTOM_FORMAT){
            throw invalid_argument("Unsupported mime data format: " + mimeType.toStdString());
        }
        
        customData = data;
    }

    bool ElementMime::hasFormat(const QString &mimeType) const {
        return mimeType == CUSTOM_FORMAT;
    }

    Element* ElementMime::elementData() const {
        return customData;
    }

    GraphFlowData::GraphFlowData(){}
    GraphFlowData::GraphFlowData(initializer_list<Pin*> vec){
        for (auto& v:vec)
            addPin(v);
    }
    GraphFlowData::GraphFlowData(vector<std::string> varNamesVec, vector<Pin*> vec){
        for (auto i=varNamesVec.size(); i-->0;)
            addVar(varNamesVec[i], vec[i]);
    }

    void GraphFlowData::setReference(){
        // can be only second operand
        if (dom == nullptr)
            throw runtime_error("Invalid flow setting: * (reference) can only be second operand; for e.g.: p1->*");
        if (pins.size() > 1)
            throw runtime_error("Invalid flow setting: * (reference) can only have one pin; got " + to_string(pins.size()));
        
        is_reference = true;
    }

    void GraphFlowData::addPin(Pin *pin){
        verifyDomain(pin);

        if(is_reference){
            throw runtime_error("Invalid flow setting: * (reference) can only be second operand; for e.g.: p1->*");
        }

        auto inserted = pins.insert(pin);
        if (!inserted.second){
            throw runtime_error("Invalid flow setting: duplicate pin name: " + pin->getName());
        }

        // add flow vars
        for (size_t j=0; j<pin->connector.varNames.size(); j++){
            if(pin->connector.varTypes[j] == WorkSpace::WsDataType::Flow){
                varNames.insert(pin->connector.varNames[j]);
            }
        }
    }

    void GraphFlowData::addVar(string const& var, Pin *pin){
        verifyDomain(pin);

        pins.insert(pin);

        auto inserted = varNames.insert(var);
        if (!inserted.second){
            throw runtime_error("Invalid flow setting: duplicate pin var: " + var);
        }
    }

    void GraphFlowData::verifyDomain(Pin *pin){
        if (dom == nullptr){
            // first pin
            dom = pin->getDomain();
        }else{
            // verify they match
            if (dom->typeId != pin->getDomain()->typeId){
                ostringstream errTxt;
                errTxt << "Element:"<<pin->getOwner()->getName()<<" flows must be within same domain!";
                throw runtime_error(errTxt.str());
            }
        }
    }

    bool GraphFlowData::isEmpty(){
        return pins.empty();
    }
}
