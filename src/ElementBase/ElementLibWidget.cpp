#include "ElementLibWidget.h"

namespace ElementBase{
    ElementWidgetLib::ElementWidgetLib(Element *owner): owner(owner){
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(0);

        // name label
        QLabel *name = new QLabel(QString::fromStdString(owner->getName()));
        name->setAlignment(Qt::AlignCenter);
        name->setStyleSheet("background: white;");

        // Element body
        QLabel *elemView = new QLabel(); // container for GraphicsScene
        elemView->setPixmap(owner->getImage());
        elemView->setAlignment(Qt::AlignCenter);
        //elemView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        // hover effect
        // selectedEffect = new QGraphicsDropShadowEffect();
        // selectedEffect->setOffset(-1, -1);
        // selectedEffect->setColor(Qt::darkBlue);
        setGraphicsEffect(nullptr);

        // add pins
        // NOP, lib view shouldn't have pins

        layout->addWidget(elemView);
        layout->addWidget(name);

        // this
        setLayout(layout);
        layout->setContentsMargins(0,0,0,0);
    }

    void ElementWidgetLib::mousePressEvent(QMouseEvent *event) {
        // drag anchor
        dragAnchor = event->pos();
    }

    void ElementWidgetLib::mouseMoveEvent(QMouseEvent *event) {
        // detect drag
        if ((event->pos()-dragAnchor).manhattanLength() > 5){
            // start drag
            QDrag *drag = new QDrag(this);
            ElementMime *data = new ElementMime();
            data->setData(CUSTOM_FORMAT, owner);
            drag->setMimeData(data);
            drag->setPixmap(owner->getImage());

            drag->exec();
        }
    }

    void ElementWidgetLib::mouseDoubleClickEvent(QMouseEvent *event) {
        Q_UNUSED(event);
        // create elem in active Subsystem
        //catElemCreation();
    }

    void ElementWidgetLib::mouseReleaseEvent(QMouseEvent *event) {
        event->accept();
        if(event->button() == Qt::RightButton){
            QMenu menu;
            // CATALOG MENU
            QAction *menuItem = new QAction("Add", this);
            connect(menuItem, &QAction::triggered, [&](){
                //catElemCreation();
            });
            menu.addAction(menuItem);

            menu.move(event->globalPosition().toPoint());
            menu.exec();
        }
    }
}
