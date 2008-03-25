#include "imageBay.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <math.h>

ImageBay::ImageBay(QGraphicsItem * parent)
  :QGraphicsPathItem(parent)
{ 
}

ImageBay::ImageBay(Qt::DockWidgetArea position, QString name, QPen pen,QGraphicsItem * parent)
  :QGraphicsPathItem(parent)
{
  QRectF bound;
  qreal startAngle;
  qreal sweepLength = 360;
  QPainterPath path;
  if(position == Qt::LeftDockWidgetArea){
    path.moveTo(-10000,-5250);
    qDebug("Left Bay");
    bound = QRectF(-10000,-10500,20000,10000);
    startAngle = 180;
  }
  path.arcTo(bound,startAngle,sweepLength);
  path.closeSubpath();
  
  setPen(pen);
  setBrush(pen.brush());
  setData(0,name);
  setPath(path);
}
