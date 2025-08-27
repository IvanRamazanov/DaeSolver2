#include "../Painter.h"

namespace dae_solver{
    AxisView::AxisView(QGraphicsScene *scene, bool isHoriz):
        horizontal(isHoriz),
        scene(scene)
    {
        // set default margins
        if(horizontal){
            axMargin = 40;
        }else{
            axMargin = 90;
        }
    }

    void AxisView::createGrid(){
        createLabel(addTickLine(minV), minV);

        if (!horizontal){
            if(maxV > 0.0 && minV < 0.0){
                createLabel(addTickLine(0), 0);
            }
        }

        createLabel(addTickLine(maxV), maxV);
    }

    double AxisView::addTickLine(double value){
        // Note: line will go over viewBox by dashLen/2
        if (horizontal){
            double x = viewBox.left() + (viewBox.right()-viewBox.left())*(value-minV)/(maxV-minV),
                    y1 = viewBox.top() - dashLen/2,
                    y2 = y1 + dashLen;
            children.append(new QGraphicsLineItem(x, y1, x, y2));
            return x;
        }else{
            double y =  viewBox.bottom() - (viewBox.bottom()-viewBox.top())*(value-minV)/(maxV-minV),
                    x1 = viewBox.right() - dashLen/2,
                    x2 = x1 + dashLen;
            children.append(new QGraphicsLineItem(x1, y, x2, y));
            return y;
        }
    }

    void AxisView::createLabel(double centerPos, double val){
        auto lbl = new QGraphicsTextItem(QString("%1").arg(val)); // arg(val, 0, 'f', 3) ? arg(val, 6, 'e', 4)
        double x,y;
        if (horizontal){
            y = viewBox.top() + dashLen/2 + labelMargin;
            x = centerPos - lbl->boundingRect().width()/2;
        }else{
            x = viewBox.right() - dashLen/2 - labelMargin - lbl->boundingRect().width();
            y = centerPos - lbl->boundingRect().height()/2;
        }
        lbl->setPos(x, y);
        children.append(lbl);
    }

    void AxisView::drawAxes(QRectF const& viewBox, double minV, double maxV){
        this->minV = minV;
        this->maxV = maxV;
        this->viewBox = viewBox;

        // div by 0 protection
        if (abs(minV-maxV) < 1e-8){
            this->minV = minV-0.5;
            this->maxV = maxV+0.5;
        }

        // clear scene
        for (auto& gi:children){
            scene->removeItem(gi);
            delete gi;
        }
        children.clear();

        if (horizontal){
            // draw main ax line
            children.append(new QGraphicsLineItem(QLineF(viewBox.topLeft(), viewBox.topRight())));
        }else{
            // draw main ax line
            children.append(new QGraphicsLineItem(QLineF(viewBox.topRight(), viewBox.bottomRight())));
        }
        createGrid();

        for (auto& l:children){
            scene->addItem(l);
        }
    }

    double AxisView::getMaxV(){
        return maxV;
    }

    double AxisView::getMinV(){
        return minV;
    }

    int AxisView::getMargin(){
        return axMargin;
    }
}
