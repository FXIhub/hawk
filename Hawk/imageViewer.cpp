#include "imageViewer.h"
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include "imageItem.h"

ImageViewer::ImageViewer(QGraphicsView * view,QWidget * parent)
  :QGraphicsScene(parent)
{
  graphicsView = view;
  addEllipse(QRect(-20000,-20000,40000,40000),QPen(),QBrush(Qt::blue));
}

void ImageViewer::mouseMoveEvent(QGraphicsSceneMouseEvent * event){
  ImageItem * it = (ImageItem *) itemAt(event->scenePos());
  if(it){
    QPointF pos = it->mapFromScene(event->scenePos());
    qDebug("Pixel %f %f",pos.x(),pos.y());
  }
  if(event->buttons() & Qt::LeftButton){
    if(itemAt(event->scenePos())){
      ((ImageItem *)itemAt(event->scenePos()))->mouseMove(event);
      return;
    }
    QList<QGraphicsItem *> it = items();
    for(int i = 0; i < it.size(); i++){
      ((ImageItem *)it[i])->mouseMove(event);
    }
  }else if(event->buttons() & Qt::RightButton){  
    QPointF mouse_mov = event->screenPos()-event->lastScreenPos();  
    qreal speed = 0.005;
    qreal scale = 1-mouse_mov.y()*speed;
    graphicsView->scale(scale, scale);
  }
}
void ImageViewer::wheelEvent( QGraphicsSceneWheelEvent * event ){
  qreal speed = 0.0005;
  qreal scale = 1+event->delta()*speed;
  graphicsView->scale(scale, scale);
}
