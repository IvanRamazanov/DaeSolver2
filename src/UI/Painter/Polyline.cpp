#include "../Painter.h"

namespace dae_solver{
    Polyline::Polyline(){
        setZValue(-1);
    }

    void Polyline::addPoint(double x, double y)  {
        polyline.append(QPointF(x,y));
    }

    void Polyline::setColor(QColor const& clr){
        lineColor = clr;
    }

    void Polyline::clear() {
        polyline.clear();
    }

    void Polyline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        Q_UNUSED(option);
        Q_UNUSED(widget);

        painter->setPen(lineColor);
        for (auto i=polyline.size(); i-->1;)
            painter->drawLine(polyline[i], polyline[i-1]);
    }

    QRectF Polyline::boundingRect() const {
        return borders;
    }
}
