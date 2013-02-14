#include "graphwidget.h"

Path::Path(const QVector<double> &x, const QVector<double> &y)
{
        Path();
        int N = std::min(x.size(), y.size());
        for (int i=0; i<=N; i++)
        {
                v_data.append(QPointF(x[i], y[i]));
        }
}

Path::Path() : s_color(Qt::red), s_target(0)
{
}

GraphWidget::GraphWidget(QWidget *parent/*=NULL*/) : QWidget(parent), trans(0, 0), s_scale(defaultScale)
{
        this->createTime= QDateTime::currentMSecsSinceEpoch()/1000;
        t1 = new Path();
        t1->addPoint(QPointF(0,0));
        t1->setColor(QColor(255,7,00));
        t2 = new Path();
        t2->addPoint(QPointF(0,0));
        t2->setColor(QColor(255,250,0));
        t3 = new Path();
        t3->addPoint(QPointF(0,0));
        t3->setColor(QColor(0,209,14));
}

bool GraphWidget::addPath(Path* p)
{
        if (!paths.contains(p))
        {
                paths.insert(p);
                return true;
        }
        else
                return false;
}

Path* GraphWidget::takePath(Path *p)
{
        paths.remove(p);
        return p;
}

void GraphWidget::clearPaths(bool del /* = no */)
{
        if (del)
                foreach (Path* p, paths)
                        delete p;
        paths.clear();
}

QSize GraphWidget::minimumSizeHint() const
 {
     return QSize(50, 50);
 }

 QSize GraphWidget::sizeHint() const
 {
     return QSize(80, 80);
 }

void GraphWidget::paintEvent(QPaintEvent *event)
{
        int i=0;
        QPainter p(this);       
        p.fillRect(this->rect(), QColor(78,78,127));
        p.setRenderHint(QPainter::Antialiasing, false);
        //draw t1 //could make some loop but mehh...

        //draw horizontal scale
        p.setPen(QPen(QColor(179,179,179),1));
        for(int i=0; i<=300; i+=50){
            p.drawLine( 1, yToCanvY(i), this->width(), yToCanvY(i) );
        }
        for(int i=0; i<=120; i+=10){
            p.drawLine( qRound(xToCanvX(i)-trans.x()), 1, qRound(xToCanvX(i)-trans.x()), this->height() );
        }

        p.setPen(QPen(QColor(166,4,0),2));
        p.drawLine( 1, yToCanvY(t1->target()), this->width(), yToCanvY(t1->target()) );

        p.setPen(QPen(QColor(166,163,0),2));
        p.drawLine( 1, yToCanvY(t2->target()), this->width(), yToCanvY(t2->target()) );

        p.setPen(QPen(QColor(0,130,9),2));
        p.drawLine( 1, yToCanvY(t3->target()), this->width(), yToCanvY(t3->target()) );
        p.setRenderHint(QPainter::Antialiasing, true);

        if (t1->size()>0){
            for (i=1; i<t1->size(); i++)
            {
                p.setPen(QPen(QColor(qMin(255,(int)((double)255*((*t1)[i-1].y()/(t1->target()+1)))),0,255-qMin(255,(int)((double)255*((*t1)[i-1].y()/(t1->target()+1))))),2));
                p.drawLine( xToCanvX((*t1)[i-1].x()), yToCanvY((*t1)[i-1].y()),
                            xToCanvX((*t1)[i].x()), yToCanvY((*t1)[i].y()) );
            }
        }

        if (t2->size()>0){
            for (i=1; i<t2->size(); i++)
            {
                    p.drawLine( xToCanvX((*t2)[i-1].x()), yToCanvY((*t2)[i-1].y()),
                                            xToCanvX((*t2)[i].x()), yToCanvY((*t2)[i].y()) );
            }
        }

        if (t3->size()>0){
            for (i=1; i<t3->size(); i++)
            {
                p.setPen(QPen(QColor(0,qMin(255,(int)((double)255*((*t3)[i].y()/(t3->target()+1)))),255-qMin(255,(int)((double)255*((*t3)[i].y()/(t3->target()+1))))),2));
                    p.drawLine( xToCanvX((*t3)[i-1].x()), yToCanvY((*t3)[i-1].y()),
                                            xToCanvX((*t3)[i].x()), yToCanvY((*t3)[i].y()) );
            }
        }

        if(xToCanvX((*t2)[i-1].x())>this->width()){
            trans.setX( (((*t2)[i-1].x()*((double)width()/(double)120))-(width()))*-1 );
        }

        p.end();
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
        prevPos = event->pos();
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
        trans += event->pos() - prevPos;
        prevPos = event->pos();
        repaint();
}


void GraphWidget::addMeasurment(double temp1, double temp2, double temphb, qint64 time){
    t1->addPoint(QPointF(time-this->createTime,temp1));
    t2->addPoint(QPointF(time-this->createTime,temp2));
    t3->addPoint(QPointF(time-this->createTime,temphb));
    repaint();
}
void GraphWidget::setTargets(double temp1, double temp2, double temp3){
    if(temp1>=0)
        t1->setTarget(temp1);
    if(temp2>=0)
        t2->setTarget(temp2);
    if(temp3>=0)
        t3->setTarget(temp3);
    repaint();
}
