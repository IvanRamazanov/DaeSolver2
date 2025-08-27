/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include <QPainter>
#include <QLineF>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QEvent>
#include <QMouseEvent>
#include "../MathPack/MatrixEqu.h"

namespace dae_solver{
/**
 *
 * @author ramazanov_im
 */

class Painter;
class AxisView;

class Polyline : public QGraphicsItem {
    QRectF borders;
    QColor lineColor;
    QList<QPointF> polyline;

    public:
        Polyline();

        void addPoint(double x, double y);

        void setColor(QColor const& clr);

        void clear();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

        QRectF boundingRect() const override;
};

class ZoomRectangle : public QGraphicsItem {
    QList<QLineF> shape;
    QPen pen;
    double stX=0,stY=0,enX=0,enY=0;

    private:
        void updateLines();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    public:
        ZoomRectangle();

        const QList<QLineF>& get();

        double getEndX();

        double getEndY();

        void setEnd(double eX, double eY);

        void setStart(double sX, double sY);

        QRectF boundingRect() const override;
};

class PlotView : public QGraphicsView {
    Painter *owner;
    QGraphicsScene *plotScene;
    vector<unique_ptr<Polyline>> lines;
    ZoomRectangle zoomRect;
    QGraphicsLineItem onDragLin1, onDragLinStr, onDragLin2;
    double maxY,natMaxY,
            minY,natMinY,
            maxT,natMaxT,
            minT,natMinT;
    double ax,ay,
            rx,ry;
    bool wasPressed = false;
    // plot data
    vector<vector<double>> data;
    vector<double> time;
    // axes
    AxisView *xAx, *yAx;
    
    public:
        PlotView(Painter *owner);

        ~PlotView();

    private:

        /**
         * Set scene rectangle based on current min/max Ax values
         */
        void updateZoom();
        void resetZoom();

        QMenu* contextMenu();

    protected:
        virtual void mousePressEvent(QMouseEvent *event) override;

        virtual void mouseMoveEvent(QMouseEvent *event) override;

        virtual void mouseReleaseEvent(QMouseEvent *event) override;

        virtual void resizeEvent(QResizeEvent *event) override;

    public:
        /**
         * Clears the scene and draws lines
         */
        void draw();

        // for data picker
        vector<double> getValue(double x0);

    friend class Painter;
};

class ValueRepresenter : public QGraphicsItem {
    QLineF line;
    QLabel *timeLabel, *coordLabel;
    double x=0,pX=0,pY=0;
    vector<double> y;
    Painter *owner;
    //EventHandler<ViewDataEvent> mpressed;

    ValueRepresenter(Painter *owner);

    ~ValueRepresenter();

    void init();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    void show(double x, vector<double> const& y);

};

class AxisView {
    private:
        QList<QGraphicsItem*> children;
        double dashLen = 6, // size of tick line
                labelMargin = 5; // margin between end of tick line and tick labels
        int axMargin; // how much space an axis takes
        bool horizontal = true;
        QGraphicsScene *scene; // scene where plot is drawn
        QRectF viewBox;
        double minV=0, maxV=1;


    private:
        void createGrid();

        double addTickLine(double value);

        void createLabel(double centerPos, double val);
        
    public:
        AxisView(QGraphicsScene *scene, bool isHoriz);

        /**
         * @param viewBox - rect that limits this axis (not the whole view!)
         */
        void drawAxes(QRectF const& viewBox, double minV, double maxV);

        double getMaxV();

        double getMinV();

        int getMargin();
};

class Painter : public QWidget {
    Q_OBJECT

    private:
        PlotView *plotView = nullptr;
        double M = 0.5;
        int zoomType=0;
        // EventHandler dragEvent;
        //static final EventType<ViewDataEvent> AZAZA=new EventType<>(Event.ANY,"AZAZA");

    private:
        void save();

        QMenuBar* initMenu();

        void initGui();

    public:
        Painter();

        void plot(vector<vector<double>> const& data, vector<double> const& time, double minY, double maxY);

        double getMinT();

        double getMaxT();

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

    friend class ValueRepresenter;
    friend class PlotView;
};
}

#endif
