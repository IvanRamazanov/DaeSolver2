#include "ElementWidget.h"

namespace ElementBase{

    ElemBodyWrapper::ElemBodyWrapper(Element *owner): owner(owner) {
        // body image
        view = new View(owner);
        view->setParentItem(this);
        realSize = view->boundingRect();
        // Note: QGraphicsPixmapItem's rect doesn't include scale, so apply it manually
        realSize.setHeight(realSize.height() * view->scale());
        realSize.setWidth(realSize.width() * view->scale());

        // set size
        realSize.setTopLeft(QPointF(0,0));
        setMinimumSize(realSize.width(), realSize.height());
        setMaximumSize(realSize.width(), realSize.height());

        // for rotation
        setTransformOriginPoint(realSize.width()/2, realSize.height()/2);
        setRotation(owner->getRotation());
    }

    void ElemBodyWrapper::addPin(Pin* pin){
        // add pins
        auto pv = pin->getView();
        pv->setParentItem(this);

        // update realSize
        auto chRect = pv->boundingRect();
        chRect.moveTo(pv->pos());
        realSize = realSize.united(chRect);

        // update size
        realSize.setTopLeft(QPointF(0,0));
        setMinimumSize(realSize.width(), realSize.height());
        setMaximumSize(realSize.width(), realSize.height());

        // for rotation
        setTransformOriginPoint(realSize.width()/2, realSize.height()/2);

        update();
    }



    ElementWidget::ElementWidget(Element *owner): owner(owner){
        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
        layout->setContentsMargins(0,0,0,0);

        // name label
        nameLabel = new QLabel(QString::fromStdString(owner->getName()));
        nameLabel->setAlignment(Qt::AlignCenter);
        auto nameProxy = new QGraphicsProxyWidget();
        nameProxy->setWidget(nameLabel);
        nameProxy->setZValue(1);

        // Element body
        elemBody = new ElemBodyWrapper(owner); // proxy for LinearLayout

        layout->addItem(elemBody);
        layout->setAlignment(elemBody, Qt::AlignHCenter);
        layout->addItem(nameProxy);
        setLayout(layout);

        // positioning
        this->moveBy(owner->getX(), owner->getY());
    }

    ElementWidget::~ElementWidget(){
        //owner->_view = nullptr;
        elemBody->setGraphicsEffect(nullptr); // also frees the memory
    }

    QMenu* ElementWidget::initMenu(){
        QMenu *ret = new QMenu();
        // DEFAULT MENU
        QAction *menuItem = new QAction("Delete");
        QWidget::connect(menuItem, &QAction::triggered, [&](){owner->getSystem()->removeElement(owner);});
        ret->addAction(menuItem);

        menuItem = new QAction("Parameters");
        QWidget::connect(menuItem, &QAction::triggered, [&](){
            auto wind = owner->openDialogStage();
            wind->show();
        });
        ret->addAction(menuItem);

        menuItem = new QAction("Rotate");
        QWidget::connect(menuItem, &QAction::triggered, [&](){owner->rotate();});
        ret->addAction(menuItem);

        menuItem = new QAction("Rename");
        QWidget::connect(menuItem, &QAction::triggered, [&](){showRenameWindow();});
        ret->addAction(menuItem);
        
        return ret;
    }

    void ElementWidget::showRenameWindow(){
        QWidget *st = new QWidget();
        st->setAttribute(Qt::WA_DeleteOnClose);
        st->setWindowModality(Qt::ApplicationModal);
        QVBoxLayout *root = new QVBoxLayout();

        // description
        root->addWidget(new QLabel("Insert new name"));

        // edit box
        QLineEdit *newName = new QLineEdit(QString::fromStdString(owner->getName()));
        root->addWidget(newName);

        // button group
        QDialogButtonBox *br = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        br->setOrientation(Qt::Horizontal);
        QWidget::connect(br, &QDialogButtonBox::accepted, [&](){
            owner->setName(newName->text().toStdString());
            st->close();
        });
        QWidget::connect(br, &QDialogButtonBox::rejected, [&](){
            st->close();
        });
        root->addWidget(br);

        st->setLayout(root);
        st->show();
        //st->raise();
    }

    void ElementWidget::setRotation(qreal angle){
        elemBody->setRotation(angle);
    }

    void ElementWidget::addPin(Pin* pin){
        elemBody->addPin(pin);
    }

    double ElementWidget::getBodyHeight(){
        return elemBody->realSize.height();
    }

}
