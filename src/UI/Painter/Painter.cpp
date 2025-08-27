#include "../Painter.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QActionGroup>
#include <QAction>
#include <QPrinter>

namespace dae_solver{

    void ZoomRectangle::updateLines(){
        shape[0].setLine(stX, stY, stX, enY);
        shape[1].setLine(stX, enY, enX, enY);
        shape[2].setLine(enX, enY, enX, stY);
        shape[3].setLine(enX, stY, stX, stY);

        update();
    }

    void ZoomRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        painter->setPen(pen);
        painter->drawLines(shape);
    }

    ZoomRectangle::ZoomRectangle(){
        pen.setColor(Qt::black);
        pen.setWidth(1);
        pen.setDashPattern({4,4});

        // lines
        //1
        QLineF lin(stX, stY, stX, enY);
        shape.append(lin);
        //2
        lin=QLineF(stX, enY, enX, enY);
        shape.append(lin);
        //3
        lin = QLineF(enX, enY, enX, stY);
        shape.append(lin);
        //4
        lin=QLineF(enX, stY, stX, stY);
        shape.append(lin);
    }

    const QList<QLineF>& ZoomRectangle::get(){
        return shape;
    }

    double ZoomRectangle::getEndX(){
        return enX;
    }

    double ZoomRectangle::getEndY(){
        return enY;
    }

    void ZoomRectangle::setEnd(double eX, double eY){
        enX = eX;
        enY = eY;
    }

    void ZoomRectangle::setStart(double sX, double sY){
        stX = sX;
        stY = sY;
    }

    QRectF ZoomRectangle::boundingRect() const {
        return QRectF(stX, stY, stX, enY); // pen width?
    }



        PlotView::PlotView(Painter *owner): owner(owner){
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            plotScene = new QGraphicsScene();
            setScene(plotScene);

            xAx = new AxisView(plotScene, true);
            yAx = new AxisView(plotScene, false);

            onDragLin1.setVisible(false);
            onDragLinStr.setVisible(false);
            onDragLin2.setVisible(false);
            plotScene->addItem(&onDragLin1);
            plotScene->addItem(&onDragLinStr);
            plotScene->addItem(&onDragLin2);

            zoomRect.setVisible(false);
            plotScene->addItem(&zoomRect);

            setMinimumSize(300, 200);
        }

        PlotView::~PlotView(){
            delete plotScene;
        }

        /**
         * Set scene rectangle based on current min/max Ax values
         */
        void PlotView::updateZoom(){
            // TODO !
            //setSceneRect(0,0, 100, 100);
        }

        QMenu* PlotView::contextMenu() {
            QMenu *contMenu = new QMenu();
            QAction *zoomOut = new QAction("Zoom Out");
            connect(zoomOut, &QAction::triggered, [this](){
                switch(owner->zoomType){
                    case 1:
                    {
                        double dx = (maxT-minT)/2;
                        maxT += dx;
                        minT -= dx;
                        updateZoom();
                        break;
                    }
                    case 2:
                    {
                        double dy = (maxY-minY)/2;
                        maxY += dy;
                        minY -= dy;
                        updateZoom();
                        break;
                    }
                    case 3:
                    {
                        double  dx = (maxT-minT)/2,
                                dy = (maxY-minY)/2;
                        maxT += dx;
                        minT -= dx;
                        maxY += dy;
                        minY -= dy;
                        updateZoom();
                    }
                }

            });
            QAction *zoomReset=new QAction("Reset Scale");
            connect(zoomReset, &QAction::triggered, [this](){
                maxT = natMaxT;
                minT = natMinT;
                maxY = natMaxY;
                minY = natMinY;
                updateZoom();
            });
            QAction *showValue=new QAction("Show Value");
            connect(showValue, &QAction::triggered, [this](){
                // TODO ???
            });
            QAction *print=new QAction("Print");
            connect(print, &QAction::triggered, [this](){
                owner->save();
            });

            QAction *importData=new QAction("Import data");
            connect(importData, &QAction::triggered, [this](){
                QString fileName = QFileDialog::getSaveFileName(nullptr,
                    tr("Save as..."), "", tr("Text (*.txt)"));
                if(!fileName.isNull()){
                    QFile f(fileName);
                    if (f.open(QIODeviceBase::ReadOnly | QIODevice::Text)){
                        QTextStream sc(&f);
                        //useDelimiter("\r\n"); ???

                        // init arrays
                        QString line=sc.readLine(); // header with names
                        if (!line.isNull()){
                            // file has content

                            int idx = line.indexOf(' ');
                            time.clear();
                            line = line.sliced(idx+1);
                            data.clear();
                            idx = line.indexOf(' ');
                            while(idx != -1){
                                data.push_back(vector<double>());
                                line = line.sliced(idx+1);
                                idx = line.indexOf(' ');
                            }

                            maxY = std::numeric_limits<double>::min();
                            minY = std::numeric_limits<double>::max();
                            //read data
                            while(!sc.atEnd()){
                                line = sc.readLine();
                                idx = line.indexOf(' ');
                                double t = line.first(idx).toDouble();
                                time.push_back(t);
                                line = line.sliced(idx+1);
                                int i = 0;
                                idx = line.indexOf(' ');
                                while(idx != -1){
                                    double val = line.first(idx).toDouble();
                                    maxY = max(maxY, val);
                                    minY = min(minY, val);
                                    data.at(i++).push_back(val);
                                    line = line.sliced(idx+1);
                                    idx = line.indexOf(' ');
                                }
                            }
                            double mY= maxY+(maxY-minY)*0.015,
                            miY = minY-(maxY-minY)*0.015;
                            natMaxY = mY;
                            natMinY = miY;
                            natMinT = time.at(0);
                            natMaxT = time.at(time.size()-1);
                            resetZoom();
                            draw();
                        }
                    }
                }
            });
            contMenu->addActions({zoomOut,zoomReset,showValue,print,importData});

            return contMenu;
        }

        void PlotView::mousePressEvent(QMouseEvent *event) {
            if(event->button() == Qt::LeftButton)
                if(owner->zoomType != 0){
                    ax = event->pos().x();
                    ay = event->pos().y();
                    wasPressed = true;
                }
        }

        void PlotView::mouseMoveEvent(QMouseEvent *event) {
            if(event->button() == Qt::LeftButton)
                switch(owner->zoomType){
                    case 1: //horizontal
                        onDragLin1.setLine(ax, ay+10, ax, ay-10);
                        onDragLinStr.setLine(ax, ay, event->pos().x(), ay); // TODO mapToScene
                        onDragLin2.setLine(event->pos().x(), ay+10, event->pos().x(), ay-10);
                        setCursor(Qt::SizeHorCursor);
                        //plotter.addEventHandler(MouseEvent.MOUSE_DRAGGED,dragEvent);
                        break;
                    case 2: //vertical
                        onDragLin1.setLine(ax-10, ay, ax+10, ay);
                        onDragLinStr.setLine(ax, ay, ax, event->pos().y());
                        onDragLin2.setLine(ax-10, event->pos().y(), ax+10, event->pos().y());
                        setCursor(Qt::SizeVerCursor);
                        //plotter.addEventHandler(MouseEvent.MOUSE_DRAGGED,dragEvent);
                        break;
                    case 3: //border
                        zoomRect.setStart(ax,ay);
                        zoomRect.setEnd(event->pos().x(), event->pos().y());
                        zoomRect.setVisible(true);
                        //plotter.addEventHandler(MouseEvent.MOUSE_DRAGGED,dragEvent);
                }
        }

        void PlotView::mouseReleaseEvent(QMouseEvent *event) {
            setCursor(Qt::ArrowCursor);
            // contextmenu!
            if(event->button() == Qt::RightButton){
                QMenu *menu = contextMenu();
                menu->move(event->globalPosition().toPoint());
                menu->exec();
                delete menu;
            }else{
                if(wasPressed){
    //                if(rx<0.0) rx=0.0;
    //                if(ry<0.0) ry=0.0;
                    ax=ax*(maxT-minT)/plotScene->width()+minT;
                    ay=-ay*(maxY-minY)/plotScene->height()+maxY;
                    switch(owner->zoomType){
                        case 1:
                            rx=onDragLin2.line().x2();
                            ry=onDragLin2.line().y2();
                            rx=rx*(maxT-minT)/plotScene->width()+minT;
                            ry=-ry*(maxY-minY)/plotScene->height()+maxY;

                            if(rx!=ax){ // horizontal drag
                                minT=min(ax, rx)-abs(ax-rx)*0.015;
                                maxT=max(ax, rx)+abs(ax-rx)*0.015;
                                updateZoom(); // TODO just change sceneRect?
                            }
                            onDragLin1.setVisible(false);
                            onDragLinStr.setVisible(false);
                            onDragLin2.setVisible(false);
                            break;
                        case 2:
                            rx=onDragLin2.line().x2();
                            ry=onDragLin2.line().y2();
                            rx=rx*(maxT-minT)/plotScene->width()+minT;
                            ry=-ry*(maxY-minY)/plotScene->height()+maxY;
                            if(ry!=ay){
                                maxY=max(ry,ay)+abs(ry-ay)*0.015;
                                minY=min(ry,ay)-abs(ry-ay)*0.015;
                                updateZoom();
                            }
                            onDragLin1.setVisible(false);
                            onDragLinStr.setVisible(false);
                            onDragLin2.setVisible(false);
                            break;
                        case 3:
                            rx=zoomRect.getEndX();
                            ry=zoomRect.getEndY();
                            rx=rx*(maxT-minT)/plotScene->width()+minT;
                            ry=-ry*(maxY-minY)/plotScene->height()+maxY;
                            if(ax!=rx && ay!=ry){
                                minT = min(ax, rx)-abs(ax-rx)*0.015;
                                maxT = max(ax, rx)+abs(ax-rx)*0.015;
                                maxY = max(ry,ay)+abs(ry-ay)*0.015;
                                minY = min(ry,ay)-abs(ry-ay)*0.015;
                                updateZoom();
                            }
                            zoomRect.setVisible(false);
                    }
                    
                    //plotter.removeEventHandler(MouseEvent.MOUSE_DRAGGED, dragEvent);
                }
            }

            wasPressed = false;
        }

        void PlotView::resizeEvent(QResizeEvent *event) {
            Q_UNUSED(event);
            draw();
            updateZoom();
        }

        void PlotView::resetZoom(){
            // reset view zoom to natural values

            minT = natMinT;
            maxT = natMaxT;
            if(natMaxY-natMinY < 0.000001){
                maxY = natMaxY+1.0;
                minY = natMinY-1.0;
            }else{
                maxY = natMaxY+(natMaxY-natMinY)*0.015;
                minY = natMinY-(natMaxY-natMinY)*0.015;
            }
        }

        /**
         * Clears the scene and draws lines
         */
        void PlotView::draw(){
            // get plottable area
            auto sceneBox = geometry().translated(-1*geometry().topLeft());
            // add margins
            auto plotArea = sceneBox.marginsRemoved(QMargins(0, 10, 30, 0));

            auto yAxBox = plotArea.adjusted(0,0, 0, -xAx->getMargin());
            yAxBox.setWidth(yAx->getMargin());
            auto xAxBox = plotArea.adjusted(yAx->getMargin(), plotArea.height()-xAx->getMargin(), 0, 0);
            // qDebug() << "Geometry: " << geometry(); 
            // qDebug() << "Scene: " << sceneBox;
            // qDebug() << "Plot area: " << plotArea;
            // qDebug() << "yAx box: " << yAxBox;
            // qDebug() << "xAx box: " << xAxBox;

            // draw axes
            if(data.empty() || time.empty()){
                yAx->drawAxes(yAxBox, 0, 1);
                xAx->drawAxes(xAxBox, 0, 1);
                setSceneRect(sceneBox);
                return;
            }
            
            yAx->drawAxes(yAxBox, minY, maxY);
            xAx->drawAxes(xAxBox, minT, maxT);

            // clear lines
            for (auto& l:lines){
                plotScene->removeItem(l.get());
            }
            lines.clear();

            //double signalMax, signalMin;
            QRect plotBox(yAxBox.topRight(), xAxBox.topRight());
            double y_scale = plotBox.height()/(maxY-minY),
                    x_scale = plotBox.width()/(maxT-minT);
                    //maxY_M = maxY*yScale, // ??
                    //minY_M = minY*yScale,
                    //y_offset = ((maxY_M+minY_M)/2.0 - plotBox.height()/2.0),
                    //x_offset = minT*xMaxRestriction/(maxT-minT);

            // draw lines
            for(size_t k=0; k<data.size(); k++){
                if(lines.size()<=k){
                    auto line = make_unique<Polyline>();
                    switch(k){
                        case 0:
                            line->setColor(Qt::red);
                            break;
                        case 1:
                            line->setColor(Qt::darkGreen);
                            break;
                        case 2:
                            line->setColor(Qt::blue);
                            break;
                        default:
                            line->setColor(QColor::fromHsvF((111+k*11)%360, 1.0, 1.0));
                            break;
                    }
                    lines.push_back(std::move(line));
                }else{
                    lines[k]->clear();
                }
                int oldX=plotBox.left()-1,
                    newX;
                int oldY=plotBox.bottom()-1,
                    newY;

                for(size_t i=0; i<data.at(k).size(); i++){
                    double y = data.at(k).at(i);
                    double x = time.at(i);
                    // // scale
                    // y = y*y_scale;
                    // x = x*x_scale;
                    // // offset
                    // y = y+dY;
                    // x = x-dX;
                    x = (x - minT) *x_scale + plotBox.left();
                    y = (minY - y) *y_scale + plotBox.bottom();
                    newX = (int) x;
                    newY = (int) y;

                    // limit displayed lines; lines should have at least 1 pixel diff (length)
                    if(oldX!=newX || oldY!=newY){
                        if(newX>=plotBox.left()){
                            lines[k]->addPoint(x, y);
                        }else{
                            // not yet in the visible are
                            // TODO calculate Y at plotBox.left() for first visible line?
                        }

                        if(newX > plotBox.right())
                            // went outside of visible area
                            break;
                        oldX=newX;
                        oldY=newY;
                    }
                }
            }

            for (const auto& l:lines)
                plotScene->addItem(l.get());

            setSceneRect(sceneBox);
        }

        vector<double> PlotView::getValue(double x0){
            if (time.empty()) return vector<double>();

            double difference=time[0];
            vector<double> out = MathPack::MatrixEqu::getColumn(data, 0);

            for(size_t i=0; i<time.size(); i++){
                if(abs(x0-time[i]) < difference){
                    difference = abs(x0-time[i]);
                    out=MathPack::MatrixEqu::getColumn(data, i);
                }else{
                    break;
                }
            }
            return out;
        }




    ValueRepresenter::ValueRepresenter(Painter *owner): owner(owner){
    }

    ValueRepresenter::~ValueRepresenter(){
        //plotter.addEventFilter(AZAZA, mpressed);
    }

    void ValueRepresenter::init(){
        // mpressed= (vde)->{
        //     if(!data.isEmpty()){
        //         pX=vde.getX();
        //         pY=vde.getY();

        //         double mX=vde.getX(),xMinRestriction=0.0,xMaxRestriction=plotter.getWidth(),
        //                 xScale=(maxT-minT)/(xMaxRestriction-xMinRestriction),
        //                 dX=(maxT-minT)/xMaxRestriction;
        //         line.setStartX(mX);
        //         line.setEndX(mX);
        //         line.setStartY(plotter.getHeight());
        //         line.setEndY(0);


        //         mX=mX*xScale+minT;
        //         x=mX;

        //         vector<Double> y=getValue(mX);
        //         show(mX, y);

        //     }
        // };

        //plotter.addEventFilter(AZAZA, mpressed);

        update();
    }

    void ValueRepresenter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Q_UNUSED(option);
        Q_UNUSED(widget);

        QPointF txtRef(0, 0); // local coords
        double txtHeight = 12;
        painter->drawText(txtRef, QString("time: %1").arg(x));
        for (size_t i=0; i<y.size(); i++){
            // shift line
            txtRef.setY(txtRef.y() + txtHeight);
            painter->drawText(txtRef, QString("In%1: %2").arg(i+1).arg(y[i]));
        }

        // line
        painter->drawLine(line);
    }

    void ValueRepresenter::show(double x, vector<double> const& y){
        setPos(pX, pY);
        this->x = x;
        this->y = y;

        //Rescale
        double  xMinRestriction=0.0,
                xMaxRestriction=owner->width(),
                xx=(x-owner->getMinT())*(xMaxRestriction-xMinRestriction)/(owner->getMaxT()-owner->getMinT());
        line.setP2(QPointF(xx, 0));
        line.setP1(QPointF(xx, owner->height()));

        update();
    }



    



        void Painter::save(){
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName("output.pdf"); // TODO from file dialog
            printer.setPageMargins(QMarginsF(12, 16, 12, 20), QPageLayout::Millimeter);
            printer.setFullPage(false);

            QPainter painter(&printer);
            double xscale = printer.pageRect(QPrinter::Millimeter).width() / width();
            double yscale = printer.pageRect(QPrinter::Millimeter).height() / height();
            double scale = qMin(xscale, yscale);
            painter.translate(printer.paperRect(QPrinter::Millimeter).center());
            painter.scale(scale, scale);
            painter.translate(-width()/ 2, -height()/ 2);
            render(&painter);
        }

        QMenuBar* Painter::initMenu(){
            QMenuBar *ret = new QMenuBar();

            QMenu *zoom = new QMenu("Zoom");

            QActionGroup *tg = new QActionGroup(zoom);
            tg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
            QAction *rmi1 = new QAction("Horizontal");
            rmi1->setCheckable(true);
            tg->addAction(rmi1);
            QAction *rmi2 = new QAction("Vertical");
            rmi2->setCheckable(true);
            tg->addAction(rmi2);
            QAction *rmi3 = new QAction("Area");
            rmi3->setCheckable(true);
            tg->addAction(rmi3);
            // MenuItem mi=new MenuItem("Cancel");
            // mi.setOnAction((n)->{
            //     zoomType=0;
            //     tg.selectToggle(null);
            // });
            // zoom.getItems().add(new SeparatorMenuItem());
            // zoom.getItems().add(mi);
            QWidget::connect(tg, &QActionGroup::triggered, [this, tg](QAction *newV){
                if (tg->checkedAction() == nullptr){
                    zoomType = 0;
                }else{
                    zoomType = tg->actions().indexOf(newV)+1;
                }
            });
            zoom->addActions({rmi1, rmi2, rmi3});

            //     Data
            QMenu *value = new QMenu("Data");
            QAction *rmi = new QAction("Press to show values");
            rmi->setCheckable(true);
            value->addAction(rmi);

            // TODO: show current value
            // ValueRepresenter *vr = new ValueRepresenter();
            // EventHandler<MouseEvent> mdrag=(me)->{
            //     plotter.fireEvent(new ViewDataEvent(vr,me.getX(),me.getY()));
            // };
            // EventHandler<MouseEvent> mpres=(me)->{
            //     plotter.addEventFilter(MouseEvent.MOUSE_DRAGGED, mdrag);
            //     plotter.fireEvent(new ViewDataEvent(vr,me.getX(),me.getY()));
            // };


            QWidget::connect(rmi, &QAction::triggered, [this](bool checked){
                if(checked){
                    // vr->init();
                    // plotter->addEventFilter(MouseEvent.MOUSE_PRESSED, mpres);
                    // plotter->addEventFilter(MouseEvent.MOUSE_RELEASED, me->{
                    //     plotter.removeEventFilter(MouseEvent.MOUSE_DRAGGED, mdrag);
                    // });
                    // plotter->heightProperty().addListener((ty,ol,ne)->{
                    //     vr.update();
                    // });
                    // plotter->widthProperty().addListener((ty,ol,ne)->{
                    //     vr.update();
                    // });
                }else{
                    // plotter->removeEventFilter(MouseEvent.MOUSE_PRESSED, mpres);
                    // vr.delete();
                }
            });
            // Data end

            ret->addMenu(zoom);
            ret->addMenu(value);
            return ret;
        }

        void Painter::initGui(){
            QVBoxLayout *root = new QVBoxLayout();
            root->setContentsMargins(0,0,0,0);
            setLayout(root);
            root->addWidget(initMenu());

            setBaseSize(400, 300);
            //getStylesheets().add("raschetkz/mod.css");

            plotView = new PlotView(this);
            root->addWidget(plotView);

            // dragEvent=(EventHandler<MouseEvent>)(MouseEvent e) -> {
            //     double x=e.getX();
            //     if(x<0.0)
            //         x=0.0;
            //     else if(x>plotter.getWidth())
            //         x=plotter.getWidth();
            //     double y=e.getY();
            //     if(y<0.0)
            //         y=0.0;
            //     else if(y>plotter.getHeight())
            //         y=plotter.getHeight();
            //     switch(zoomType){
            //         case 1:
            //             onDragLinStr.setEndX(x);
            //             break;
            //         case 2:
            //             onDragLinStr.setEndY(y);
            //             break;
            //         case 3:
            //             zoomRect.setEnd(x,y);
            //     }
            // };
        }

        Painter::Painter(){
            initGui();
        }

        void Painter::plot(vector<vector<double>> const& data, vector<double> const& time, double minY, double maxY){
            plotView->data = data;
            plotView->time = time;

            plotView->natMaxY = maxY;
            plotView->natMinY = minY;
            if(time.empty()){
                plotView->natMaxT = 1;
                plotView->natMinT = 0;
            }else{
                plotView->natMaxT = time.back();
                plotView->natMinT = time[0];
            }
            
            plotView->resetZoom();
            plotView->draw();
        }

        double Painter::getMinT(){
            return plotView->minT;
        }

        double Painter::getMaxT(){
            return plotView->maxT;
        }

    // TODO: do you really need custom Event class?
    // class ViewDataEvent : public QEvent {
    //     double x,y;

    //     public:
    //     ViewDataEvent() {
    //         //super(AZAZA);
    //     }


    //     ViewDataEvent(ValueRepresenter vr, double x, double y): x(x), y(y) {
    //         //super(AZAZA);
    //     }

        

    // };

}

