/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Scope.h"

using namespace ElementBase;

namespace Elements{

    class ScopeView : public ElementWidget{
        public:
            ScopeView(Scope* owner): ElementWidget(owner) {};

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
                            const auto &data = ((Scope*)owner)->data;
                            const auto &time = ((Scope*)owner)->time;

                            f.write("time ");
                            for(size_t i=0;i<data.size();i++)
                                f.write(QString("in%1 ").arg(i+1).toUtf8());
                            f.write("\n");
                            for(size_t i=0;i<data.at(0).size();i++){
                                f.write(QString("%1 ").arg(time.at(i)).toUtf8());
                                for(size_t j=0;j<data.size();j++)
                                    f.write(QString("%1 ").arg(data[j].at(i)).toUtf8());
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
                    QVBoxLayout *root = new QVBoxLayout();
                    auto dr = ((Scope*)owner)->parameters.at(0).get();

                    // data rate setting
                    auto pws = dr->getView();
                    QWidget *pr = new QWidget();
                    QHBoxLayout *l = new QHBoxLayout();
                    for(auto& v:pws){
                        l->addWidget(v);
                    }
                    pr->setLayout(l);
                    l->setContentsMargins(0,0,0,0);
                    pr->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
                    root->addWidget(pr);

                    QPushButton *btn = new QPushButton("Ok");
                    connect(btn, &QPushButton::clicked, [st, dr](){
                        dr->update();
                        st->close();
                    });
                    root->addWidget(btn);
                    st->setBaseSize(400, 300);
                    st->show();
                });
                ret->addAction(item);

                return ret;
            }
    };

    Scope::Scope(){
        id = "Scope";
        name = id;
        setImgPath(":src/data/Elements/Discrete/Display/Scope.png");

        addPin("in", QPointF(-3, 23), Domains::MATH.get(), false, Domains::ConnDirection::Input);
        input = pins[0].get();

        dr = addScalarParameter("dataRate", "Data rate, samples", 1);
    }

    void Scope::discreteInit() {
        maxVal = std::numeric_limits<double>::min();
        minVal = std::numeric_limits<double>::max();
        time.clear();
        data.clear();
        //double *val = input->data;
        size_t valLen = input->dataSize;
        data.resize(valLen);

        // T=0 will be handled by solver
        // for(size_t i=0; i<valLen; i++){
        //     double newVal = val[i];
        //     data[i].push_back(newVal);
        //     qDebug() << "Scope. Init val:"<<newVal;

        //     if (newVal > maxVal)
        //         maxVal = newVal;
        //     if (newVal < minVal)
        //         minVal = newVal;
        // }
        // time.push_back(0.0);

        topCnt = dr->getScalarValue();
        cnt=0;
    }

    void Scope::discreteStep(double t) {
        cnt++;
        if(cnt==topCnt) {
            double *val = input->data;
            for (size_t i = 0; i < input->dataSize; i++) {
                double newVal = val[i];
                data[i].push_back(newVal);
                //qDebug() << "Scope. Add val:"<<newVal<<" at T:"<<t;

                if (newVal > maxVal)
                    maxVal = newVal;
                if (newVal < minVal)
                    minVal = newVal;
            }
            time.push_back(t);

            cnt=0;
        }
    }

    QWidget* Scope::openDialogStage() {
        dae_solver::Painter *plotter = new dae_solver::Painter();
        plotter->setWindowTitle(QString::fromStdString(getName()));
        plotter->plot(data, time, minVal, maxVal);
        return plotter;
    }

    void Scope::initView() {
        _view = new ScopeView(this);

        // add pins
        for (auto& p:pins){
            if(!p->isExternal())
                _view->addPin(p.get());
        }
    }
}

