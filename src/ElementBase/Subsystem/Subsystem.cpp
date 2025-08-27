#include "Subsystem.h"

using namespace std;
using namespace Connections;

constexpr int PIN_OFFSET = 10; // px
constexpr int PIN_STEP = 20; // px
constexpr int W_HEIGHT = 600;
constexpr int W_WIDTH = 800;

namespace ElementBase{
    constexpr auto ELEM_LIST_TAG = "Elements";
    constexpr auto WIRE_LIST_TAG = "Wires";
    constexpr auto SCENE_TAG = "Scene";
    constexpr auto WINDOW_TAG = "Window";

    Subsystem::Subsystem(){
        name = "Subsystem";
        id = SYS_ID;
        imgPath = ":/src/data/Elements/Subsystem.png";

        initStage();
    }

    void Subsystem::setSystem(Subsystem *oldSys, vector<unique_ptr<Element>> children){
        setSystem(oldSys);

        // move elements to new subsys
        auto myScene = _getScene();
        for (auto& e:children){
            // unique ptr already removed from old system
            // just add to new
            e->setSystem(this);
            elementList.emplace_back(std::move(e));
        }

        double  minX = numeric_limits<double>::max(),
                minY = numeric_limits<double>::max();

        vector<WireCluster*> blackList; // already processed wires
        // find "external" connections
        for(auto i=elementList.size(); i-->0;){
            // check pins
            for(auto& pin:elementList[i]->_getPins()){
                if(pin->isConnected()){
                    auto pinWire = pin->getConnectedMarker()->getWire();
                    if(MathPack::indexOf(blackList, pinWire) == size_t(-1)){
                        blackList.push_back(pinWire);

                        // check if has external lineMarker
                        vector<LineMarker*> external, internal;
                        for(auto& lm:pinWire->getMarkers()){
                            if(lm->getConnectedPin() != nullptr){
                                const auto& e = lm->getConnectedPin()->getOwner();
                                // check if e (pin's element) is outside of current subsystem
                                // and not inside nested subsystem (which doesn't require Pass)
                                if(MathPack::indexOf(elementList, e) == size_t(-1)
                                    && MathPack::indexOf(elementList, e->getSystem()) == size_t(-1))
                                {
                                    // elem is outside current subsystem
                                    external.push_back(lm.get());
                                }else{
                                    // inside new subsys
                                    internal.push_back(lm.get());
                                }
                            }
                        }

                        if(external.size()>0){
                            // pass through its connection (create input/output)

                            // create pass
                            Element *portElem;
                            if(pinWire->getDomain()->directional){
                                // check is source is outside new subsystem
                                bool src_outside = false;
                                for (auto& lm:external){
                                    if (lm->getDirection()==Domains::Output){
                                        src_outside = true;
                                        break;
                                    }
                                }
                                if(src_outside)
                                    portElem = makeElement("Inport");
                                else
                                    portElem = makeElement("Outport");
                            }else
                                portElem = makeElement("Port");
                            addElement(portElem);
                            auto pass = dynamic_cast<Elements::Pass*>(portElem);
                            portElem->moveTo(0, 0);

                            // create Wire
                            auto newWire = addWire(pinWire->getDomain());
                            
                            // connect Pass inner pin
                            auto freeLm = newWire->initConnection(pass->getInner());

                            // reconnect inner side
                            auto pn = internal[0]->getConnectedPin();
                            internal[0]->unPlug();
                            freeLm->plugIn(pn);

                            // outer side
                            internal[0]->plugIn(pass->getOuter());

                            // process rest of markers
                            for (auto j=internal.size(); j-->1;){
                                pn = internal[j]->getConnectedPin();
                                internal[j]->unPlug();

                                newWire->addLineMarker(pn);
                            }
                        }
                    }
                }
            }

            // find top left point
            double  x=elementList[i]->getX(),
                    y=elementList[i]->getY();
            if(x<minX)
                minX = x;
            if(y<minY)
                minY = y;
        }

        double  dx = minX-50,
                dy = minY-50;
        for(auto& elem:elementList){
            double  x = elem->getX(),
                    y = elem->getY();
            elem->moveTo(x-dx, y-dy);
        }

        moveTo(minX, minY);
    }

    Subsystem::~Subsystem(){
        clear();
        delete drawingBoard;
    }

    void Subsystem::setSystem(Subsystem *sys){
        Element::setSystem(sys);

        drawingBoard->setParent(sys->drawingBoard);
        drawingBoard->setWindowFlag(Qt::Window, true);
    }

    void Subsystem::loadState(XmlParser::XmlElement *elemInfo){
        auto v = elemInfo->find(WINDOW_TAG);
        if(v != nullptr){
            auto layout = XmlParser::split(v->text, ",");
            if (layout.size() == 2){
                windowHeight = stoi(layout[1]);
                windowWidth = stoi(layout[0]);
            }
        }

        // scene
        v = elemInfo->find(SCENE_TAG);
        if(v != nullptr){
            auto layout = XmlParser::split(v->text, ",");
            if (layout.size() == 4){
                double  x = stod(layout[0]),
                        y = stod(layout[1]),
                        w = stod(layout[2]),
                        h = stod(layout[3]);
                drawingBoard->scene->setSceneRect(QRectF(x,y,w,h));
            }
        }

        auto elems_root = elemInfo->find(ELEM_LIST_TAG);
        if (elems_root){
            auto elems = elems_root->findAll(ELEM_TAG);
            for(auto& e:elems){
                auto new_elem = makeElement(e);
                if (new_elem){
                    // Element can fail to load (version conflicts, etc.)
                    addElement(new_elem, QPointF(new_elem->getX(), new_elem->getY()));
                }
            }
        }

        auto wires_root = elemInfo->find(WIRE_LIST_TAG);
        if (wires_root){
            auto wires = wires_root->findAll(WIRE_TAG);
            for(auto& data:wires){
                // check if empty
                if (data->findAll(LM_TAG).size() == 0)
                    // skip empty clusters
                    continue;

                try{
                    wireList.push_back(make_unique<WireCluster>(this, data));
                }catch (exception &e){
                    cerr << e.what();
                    continue;
                }

                // add graphics
                auto viewList = wireList.back()->getView();
                for (auto& i:viewList){
                    drawingBoard->scene->addItem(i);
                }
            }
        }
    }

    // creates sub graphics scene
    QWidget* Subsystem::openDialogStage(){
        return drawingBoard;
    }

    XmlParser::XmlElement* Subsystem::to_xml() {
        XmlParser::XmlElement* ret = Element::to_xml();
        ret->name = "System";

        // window
        XmlParser::XmlElement* node = new XmlParser::XmlElement(WINDOW_TAG);
        node->text = format("{},{}", windowWidth, windowHeight);
        ret->append(node);

        // scene
        node = new XmlParser::XmlElement(SCENE_TAG);
        auto r = drawingBoard->scene->sceneRect();
        node->text = format("{},{},{},{}", r.x(), r.y(), r.width(), r.height());
        ret->append(node);

        node = new XmlParser::XmlElement(ELEM_LIST_TAG);
        for(auto& elem:elementList){
            node->append(elem->to_xml());
        }
        ret->append(node);

        node = new XmlParser::XmlElement(WIRE_LIST_TAG);
        for(auto& w: wireList){
            node->append(w->to_xml());
        }
        ret->append(node);

        return ret;
    }

    void Subsystem::initStage(){
        drawingBoard = new Stage(this); // TODO could be multibpe instances of opened window

        //scene->setPannable(true);
    }

    void Subsystem::initView(){
        _view = new SystemView(this);
    }

    void Subsystem::addPinView(shared_ptr<Pin> pin, bool leftSide){
        _view->addPin(pin.get());

        alignPin(pin.get(), leftSide);

        // in order for LineMarker to find external pin, it has to be in the list
        pins.push_back(pin);
    }

    vector<WireCluster*> Subsystem::getWires(){
        vector<WireCluster*> ret;
        ret.reserve(wireList.size());
        for (auto& w:wireList)
            ret.push_back(w.get());
        // recursive into sub-Subsystems
        for (auto& e:elementList)
            if(e->type() == Subsys){
                auto sp = dynamic_cast<Subsystem*>(e.get());
                for(auto& sw:sp->getWires())
                    ret.push_back(sw);
            }
        return ret;
    }

    void Subsystem::removeElement(size_t idx){
        for (auto& p:elementList[idx]->_getPins()){
            if (p->isExternal()){
                // "external" pin, remove from subsystem layout
                for(size_t j=pins.size(); j-->0;){
                    if (p == pins[j]){
                        pins.erase(pins.begin() + j);
                        break;
                    }
                }
                for(size_t j=leftSidePins.size(); j-->0;){
                    if (p.get() == leftSidePins[j]){
                        leftSidePins.erase(leftSidePins.begin() + j);
                        break;
                    }
                }
                for(size_t j=rightSidePins.size(); j-->0;){
                    if (p.get() == rightSidePins[j]){
                        rightSidePins.erase(rightSidePins.begin() + j);
                        break;
                    }
                }
            }
        }
        // clear layout
        drawingBoard->scene->removeItem(elementList[idx]->getView());
        // remove element itself
        elementList.erase(elementList.begin() + idx);
    }

    void Subsystem::removeElement(Element *item){
        // check pins
        for (auto i=elementList.size(); i-->0;){
            if (elementList[i].get() == item){
                removeElement(i);
                break;
            }
        }
    }

    void Subsystem::clear(){
        // clear the scene
        for(auto i=elementList.size(); i-->0;){
            removeElement(i);
        }

        elementList.clear();
        wireList.clear();
    }

    void Subsystem::linkBottomBar(QObject *parentBar){
        Q_UNUSED(parentBar);
        //link bottom
        //Create bottom bar and bind to parent

        // bottom.gridLinesVisibleProperty().set(true);
        // bottom.getColumnConstraints().add(new ColumnConstraints(100));
        // bottom.getColumnConstraints().add(new ColumnConstraints(100));
        // bottom.getColumnConstraints().add(new ColumnConstraints());
        // bottom.getColumnConstraints().add(new ColumnConstraints(120));

        // ProgressBar progBar=new ProgressBar();
        // progBar.progressProperty().bind(((ProgressBar)parentBar.getChildren().get(0+1)).progressProperty());
        // Label Status=new Label(),currentFile=new Label();
        // Status.textProperty().bind(((Label)parentBar.getChildren().get(1+1)).textProperty());
        // currentFile.textProperty().bind(((Label)parentBar.getChildren().get(3+1)).textProperty());
        // Button startBtn=new Button("Start"),
        //         stopBtn=new Button("Stop"),
        //         errBtn=new Button();
        // errBtn.setMaxHeight(24);
        // startBtn.disableProperty().bindBidirectional(((Button)parentBar.getChildren().get(4+1)).disableProperty());
        // startBtn.setOnAction(((Button)parentBar.getChildren().get(4+1)).getOnAction());
        // stopBtn.onActionProperty().bind(((Button)parentBar.getChildren().get(5+1)).onActionProperty());
        // stopBtn.disableProperty().bindBidirectional(parentBar.getChildren().get(5+1).disableProperty());
        // errBtn.setOnAction(((Button)parentBar.getChildren().get(6+1)).getOnAction());
        // Image gr=((ImageView)((Button)parentBar.getChildren().get(6+1)).getGraphic()).getImage();
        // errBtn.setGraphic(new ImageView(gr));
        // errBtn.visibleProperty().bind(parentBar.getChildren().get(6+1).visibleProperty());

        // bottom.add(progBar, 1, 0);
        // bottom.add(Status, 0, 0);
        // bottom.add(new Label("File: "), 2, 0);
        // bottom.add(currentFile, 3, 0);
        // bottom.add(startBtn,4,0);
        // bottom.add(stopBtn,5,0);
        // bottom.add(errBtn,6,0);

        // root.setBottom(bottom);
    }

    void Subsystem::compileTo(){
        //perform some actions
        try {
            QFileDialog filechoose;
            filechoose.setNameFilter("txt (*.txt)");
            filechoose.setWindowTitle("Choose a file");
            filechoose.setFileMode(QFileDialog::AnyFile);
            filechoose.setViewMode(QFileDialog::Detail);
            if(filechoose.exec()){
                // TODO
                //MathPackODE.DAE dae = new MathPackODE.Compiler().evalNumState(this, filechoose.selectedFiles()[0].soStdString(), true);
            }
        }catch(exception &ex){
            cerr << ex.what() << endl;
        }
    }

    QGraphicsScene* Subsystem::_getScene(){
        return drawingBoard->scene;
    }

    void Subsystem::addElement(Element *elem, QPointF const& pos){
        elem->setSystem(this);

        if (elementList.empty() && wireList.empty()){
            // scene is empty. Init geometry to correctly process elem->moveTo
            drawingBoard->scene->setSceneRect(0,0,
                                            pos.x(),
                                            pos.y());
            elem->moveTo(pos);
            // extend rect to content
            auto content = drawingBoard->scene->itemsBoundingRect(); 
            // TODO for some reason, itemsBoundingRect ignores Label; therefore +20px for now
            // TODO try wrapping Label into VBoxLayout to force non zero Rect
            drawingBoard->scene->setSceneRect(0,0, content.right(), content.bottom()+20);
        }else{
            elem->moveTo(pos);
            // merge rects (geometry and bounding rects are all 0x0, so the expensive way is the only way)
            auto content = drawingBoard->scene->itemsBoundingRect();
            drawingBoard->scene->setSceneRect(content.united(drawingBoard->scene->sceneRect()));
        }

        elementList.emplace_back(unique_ptr<Element>(elem));
        
        // adjust scene rect
        // auto r = drawingBoard->scene->sceneRect();
        // auto content_rect = drawingBoard->scene->itemsBoundingRect();
        // r.setHeight(content_rect.top() + content_rect.height());
        // r.setWidth(content_rect.left() + content_rect.width());
        // drawingBoard->scene->setSceneRect(r);
        //drawingBoard->scene->setSceneRect(drawingBoard->scene->itemsBoundingRect());
    }

    string Subsystem::getUniqueName(string const& elemName){
        size_t cnt = 1;
        string tmpName = elemName;

        // check parent Subsystem for name duplicates
        while(true){
            // try to find Elem in parent Subsys with this name
            if (findElement(tmpName) == nullptr){
                // name is free
                break;
            }else{
                // name is already used; append
                tmpName = elemName + to_string(cnt);
                cnt++;
            }
        }

        return tmpName;
    }

    vector<Element*> Subsystem::getElements(bool recursive){
        vector<Element*> ret;
        ret.reserve(elementList.size());
        for (auto& e:elementList){
            if(e->type() == Subsys){
                ret.push_back(e.get());
                if(recursive){
                    auto sp = dynamic_cast<Subsystem*>(e.get());
                    for (auto& se:sp->getElements(recursive)){
                        ret.push_back(se);
                    }
                }
            }else{
                ret.push_back(e.get());
            } 
        }
        return ret;
    }

    /**
     * Find element by its name
     * @param name
     * @return
     */
    Element* Subsystem::findElement(string const& name){
        for(auto& elem:elementList){
            if(elem->getName() == name)
                return elem.get();
        }
        return nullptr;
    }

    ElemTypes Subsystem::type(){
        return Subsys;
    }

    WireCluster* Subsystem::addWire(Domains::Domain *wireDomain) {
        wireList.push_back(make_unique<WireCluster>(this, wireDomain));
        WireCluster *newWire = wireList.back().get();

        auto viewList = newWire->getView();
        for (auto& i:viewList)
            drawingBoard->scene->addItem(i);
        return newWire;
    }

    // ------ CLASS SYSTEM_VIEW ------

    Subsystem::SystemView::SystemView(Element* owner): ElementWidget::ElementWidget(owner) {
    }

    QMenu* Subsystem::SystemView::initMenu(){
        auto ret = ElementWidget::initMenu();
        
        // additional menus
        QAction *menuItem = new QAction("Compile to...");
        QWidget::connect(menuItem, &QAction::triggered, [&](){
            static_cast<Subsystem*>(owner)->compileTo();
        });
        ret->addAction(menuItem);

        return ret;
    }

    bool Subsystem::hasElement(Element* elem){
        for (auto& e:elementList){
            if (e.get() == elem)
                return true;
        }
        return false;
    }

    void Subsystem::setName(string const& name){
        Element::setName(name);

        drawingBoard->setWindowTitle(QString::fromStdString(this->name));
    }

    Pin* Subsystem::_findPin(QPointF atPos){
        for (auto& e:elementList){
            // chech if Pin checkBox contains event point
            for (auto& p:e->_getPins()){
                auto pr = p->getView()->mapRectToScene(p->getView()->boundingRect());
                if (pr.contains(atPos) && !p->isConnected()){
                    return p.get();
                }
            }
        }

        return nullptr;
    }

    void Subsystem::removeWire(WireCluster* wire){
        MathPack::remove(wireList, wire);
    }
    
    

    // ----- CLASS SELECTION_MODEL -----

    SelectionModel::SelectionModel(){
        hide();
    }

    void SelectionModel::init(double x,double y){
        ix=x;
        iy=y;
        rect.setX(x);
        rect.setY(y);
        rect.setHeight(0);
        rect.setWidth(0);
        //rect.toFront();
        clear();
    }

    void SelectionModel::update(double x, double y){
        rect.setHeight(y-iy);
        rect.setWidth(x-ix);
        // highlight selected
    }

    void SelectionModel::confirm(vector<unique_ptr<Element>> const& elements){
        for(auto& elem:elements){
            if(collidesWithItem(elem->_view, Qt::ContainsItemShape)){
                selection.push_back(elem.get());
                selectedElements.push_back(elem.get());
                elem->setSelected(true);
            }
        }
    }

    void SelectionModel::clear(){
        for(auto& elem:selectedElements){
            elem->setSelected(false);
        }
        selection.clear();
        selectedElements.clear();
    }

    vector<Element*> SelectionModel::getSelectedElements(){
        return selectedElements;
    }

    void SelectionModel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->setBrush(QBrush(QColor::fromString("aqua")));
        painter->setOpacity(0.3);
        painter->setPen(QPen(QColor::fromString("darkgrey")));
        painter->drawRect(rect);
    }

    QRectF SelectionModel::boundingRect() const {
        QRectF ret(0,0,0,0); // TODO
        return ret;
    }

    //----- CLASS STAGE ------
    class DndScene : public QGraphicsScene {
        using QGraphicsScene::QGraphicsScene;

        protected:
            void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override {
                // empty, to prevent canceling DnD by default
                Q_UNUSED(event);
            }

        public:
            ~DndScene(){
                // scene elements own their own memory, so remove items to prevent premature deallocation
                for (auto i:items())
                    removeItem(i);
            }
    };

    Stage::Stage(Subsystem *owner): owner(owner){
        scene = new DndScene();
        selectionModel = make_unique<SelectionModel>();
        setScene(scene);
        setAcceptDrops(true);
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

    Stage::~Stage(){
        delete scene;
    }

    // void Subssytem::Stage::setOnDragDetected((MouseEvent me)->{
    //     getScrollPane().setCursor(Cursor.CLOSED_HAND);
    // }

    // void Subssytem::Stage::setOnDragOver(de->{
    //     de.acceptTransferModes(TransferMode.ANY);
    // }

    void Stage::keyReleaseEvent(QKeyEvent *event){
        if(event->key() == Qt::Key_G && (event->modifiers() & Qt::ShiftModifier)){
            // if(!selectionModel->getSelectedElements().empty()){
            //     // pack selection into new (child) subsystem
            //     owner->addElement(new Subsystem(owner, selectionModel->getSelectedElements()));
            // }

            vector<unique_ptr<Element>> selected;
            for (auto i=owner->elementList.size(); i-->0;){
                if (owner->elementList[i]->isSelected()){
                    selected.push_back(std::move(owner->elementList[i]));
                    owner->elementList.erase(owner->elementList.begin()+i);
                }
            }
            auto newSys = dynamic_cast<Subsystem*>(makeElement(SYS_ID));
            newSys->setSystem(owner, std::move(selected));
            owner->addElement(newSys, newSys->getView()->pos());
        }else{
            event->setAccepted(false);
            QGraphicsView::keyReleaseEvent(event);
        }
    }

    void Stage::mousePressEvent(QMouseEvent *mouseEvent){
        if (mouseEvent->button() == Qt::RightButton){
            setDragMode(QGraphicsView::ScrollHandDrag);
            //setDragMode(QGraphicsView::RubberBandDrag);
        }else if(mouseEvent->button() == Qt::LeftButton){
            setDragMode(QGraphicsView::RubberBandDrag);
            // if(mouseEvent->modifiers() & Qt::ControlModifier) {
            //     //getScrollPane().setPannable(false);
            //     //selectionModel->init(mouseEvent->pos().x(), mouseEvent->pos().y());
            //     setDragMode(QGraphicsView::RubberBandDrag);
            // }else{
            //     setDragMode(QGraphicsView::ScrollHandDrag);
            // }
        }

        QGraphicsView::mousePressEvent(mouseEvent);
    }

    // void Stage::mouseMoveEvent(QMouseEvent *event) {
    //     if(selectionInProgress){
    //         selectionModel->update(event->pos().x(), event->pos().y());
    //     }else{
    //         // span the view
    //     }
    // }

    void Stage::mouseReleaseEvent(QMouseEvent *event){
        if(dragMode() == QGraphicsView::RubberBandDrag) {
            // TODO implement!
            // selectionModel->confirm(owner->elementList);
            // scene->removeItem(selectionModel.get());
            // selectionInProgress = false;
            // selectionModel->clear();
        }
        setDragMode(QGraphicsView::NoDrag);

        QGraphicsView::mouseReleaseEvent(event);
    }

    void Stage::dragEnterEvent(QDragEnterEvent* event) {
        if (event->mimeData()->hasFormat(CUSTOM_FORMAT)) {
            event->acceptProposedAction();
        }
    }

    void Stage::dropEvent(QDropEvent *event){
        if (event->mimeData()->hasFormat(CUSTOM_FORMAT)){
            // int x = event->position().toPoint().x(),
            //     y = event->position().toPoint().y();
            const ElementMime* elemData = static_cast<const ElementMime*>(event->mimeData());
            Element *obj = elemData->elementData()->clone();
            owner->addElement(obj, mapToScene(event->position().toPoint()));

            //event->setAccepted(true);
            event->acceptProposedAction();
        }
    }

    // void Stage::resizeEvent(QResizeEvent *event) {
    //     auto oldRect = scene->sceneRect();
    //     oldRect.setHeight(event->size().height());
    //     oldRect.setWidth(event->size().width());
    //     scene->setSceneRect(oldRect);
    // }
}
