#include "XYGraph.h"

using namespace ElementBase;

namespace Elements{

    class ScopeView : public ElementWidget{
        public:
            ScopeView(XYGraph* owner): ElementWidget(owner) {};

        protected:
            virtual QMenu* initMenu() override {
                QMenu *ret = ElementWidget::initMenu();

                QAction *item = new QAction("Export");
                QWidget::connect(item, &QAction::triggered, [this](){
                    QString fileName = QFileDialog::getSaveFileName(nullptr, // this?
                        tr("Save as..."), "", tr("Text (*.txt)"));
                    if(!fileName.isNull()){
                        QFile f(fileName);
                        if(f.open(QIODevice::WriteOnly)){
                            const auto &data = ((XYGraph*)owner)->dataY;
                            const auto &time = ((XYGraph*)owner)->dataX;

                            f.write("time ");
                            for(size_t i=0;i<data.size();i++)
                                f.write(QString("in%1 ").arg(i+1).toUtf8());
                            f.write("\n");
                            for(size_t i=0;i<data.size();i++){
                                f.write(QString("%1 ").arg(time.at(i)).toUtf8());
                                f.write(QString("%1 ").arg(data.at(i)).toUtf8());
                                f.write("\n");
                            }
                        }
                    }
                });
                ret->addAction(item);

                item = new QAction("DataRate");
                QWidget::connect(item, &QAction::triggered, [this](){
                    QWidget *st = new QWidget();
                    st->setAttribute(Qt::WA_DeleteOnClose);
                    //QVBoxLayout *root = new QVBoxLayout();
                    //auto dr = ((XYGraph*)owner)->parameters.at(0).get();

                    // data rate setting
                    //auto pws = dr->getView();
                    // QWidget *pr = new QWidget();
                    // QHBoxLayout *l = new QHBoxLayout();
                    // for(auto& v:pws){
                    //     l->addWidget(v);
                    // }
                    // pr->setLayout(l);
                    // l->setContentsMargins(0,0,0,0);
                    // pr->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
                    // root->addWidget(pr);

                    // QPushButton *btn = new QPushButton("Ok");
                    // connect(btn, &QPushButton::clicked, [st, dr](){
                    //     dr->update();
                    //     st->close();
                    // });
                    // root->addWidget(btn);
                    st->setBaseSize(400, 300);
                    st->show();
                });
                ret->addAction(item);

                return ret;
            }
    };

    XYGraph::XYGraph(){
        id = "XYGraph";
        name = id;
        setImgPath(":src/data/Elements/Discrete/Display/XYGraph.png");

        auto p = addPin("inX", Domains::ConnDirection::Input);
        p->dataSizeSetting = 1;
        p = addPin("inY", Domains::ConnDirection::Input);
        p->dataSizeSetting = 1;

        //dr = addScalarParameter("dataRate", "Data rate, samples", 1);
    }

    void XYGraph::discreteInit() {
        inpX = pins[0]->data;
        inpY = pins[1]->data;

        maxX = std::numeric_limits<double>::min();
        maxY = std::numeric_limits<double>::min();
        minX = std::numeric_limits<double>::max();
        minY = std::numeric_limits<double>::max();
        dataX.clear();
        dataY.clear();

        topCnt = 0;//dr->getScalarValue();
        cnt=0;
    }

    void XYGraph::discreteStep(double time) {
        (void)time;
        //cnt++;
        //if(cnt==topCnt) {
            dataX.push_back(*inpX);
            if (*inpX > maxX)
                maxX = *inpX;
            if (*inpX < minX)
                minX = *inpX;

            dataY.push_back(*inpY);
            if (*inpY > maxY)
                maxY = *inpY;
            if (*inpY < minY)
                minY = *inpY;
            
            //cnt=0;
        //}
    }

    QWidget* XYGraph::openDialogStage() {
        dae_solver::Painter *plotter = new dae_solver::Painter();
        plotter->setWindowTitle(QString::fromStdString(getName()));
        plotter->plot({dataY}, dataX, minY, maxY);
        return plotter;
    }

    void XYGraph::initView() {
        _view = new ScopeView(this);

        // add pins
        for (auto& p:pins){
            if(!p->isExternal())
                _view->addPin(p.get());
        }
    }
}
