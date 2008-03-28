#include "imageBay.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <math.h>

ImageBay::ImageBay(QGraphicsItem * parent)
  :QGraphicsPathItem(parent)
{ 
}

ImageBay::ImageBay(Qt::DockWidgetArea position, QString name, QPen pen,QBrush brush ,QGraphicsItem * parent)
  :QGraphicsPathItem(parent)
{
  QRectF bound;
  QPainterPath path;
  label = new QGraphicsTextItem(this);
  QFont font = label->font();
  font.setPointSizeF(100);  
  label->setFont(font);
  label->setDefaultTextColor(pen.color());
  label->setTextWidth(2000);
  label->setHtml(QString("<center>" + name + "</center>"));
  if(position == Qt::TopDockWidgetArea){
    qDebug("Top Bay");
    path.addRect(QRectF(-1000,-3000,2000,2000));
    ((QLinearGradient*)brush.gradient())->setStart(-1000,-3000);
    ((QLinearGradient*)brush.gradient())->setFinalStop(-1000,-1000);    
    label->setPos(-1000,-2000);
  
  }
  if(position == Qt::LeftDockWidgetArea){
    qDebug("Left Bay");
    path.addRect(QRectF(-3000,-1000,2000,2000));
    ((QLinearGradient*)brush.gradient())->setStart(-3000,-1000);
    ((QLinearGradient*)brush.gradient())->setFinalStop(-3000,1000);
    label->setPos(-3000,0);
  }
  if(position == Qt::BottomDockWidgetArea){
    qDebug("Bottom Bay");
    path.addRect(QRectF(-1000,1000,2000,2000));
    ((QLinearGradient*)brush.gradient())->setStart(-2000,1000);
    ((QLinearGradient*)brush.gradient())->setFinalStop(-2000,3000);
    label->setPos(-1000,2000);
  }
	       
  setPen(pen);
  setBrush(brush);
  setData(0,"Bay");
  setPath(path);
}

QPointF ImageBay::centeredScale(qreal s,QPointF screenCenter){
  QPointF item_sc = mapFromScene(screenCenter);
  scale(s, s);
  if(label){
    // constant width pen regardless of zoom
    label->scale(1.0/s,1.0/s);
    label->setTextWidth(label->textWidth()*s);
    QFont f = label->font();
    f.setPointSizeF(f.pointSizeF()*s);  
    label->setFont(f);
  }
  QPointF mov = mapFromScene(screenCenter)-item_sc;
  translate(mov.x(),mov.y());
  return QPointF(transform().m11(),transform().m22());
}
