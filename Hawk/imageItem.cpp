#include "imageItem.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <math.h>

ImageItem::ImageItem(QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
}

ImageItem::ImageItem(const QPixmap & pix, QGraphicsItem * parent )
  :QGraphicsPixmapItem(pix,parent)
{ 
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event){
  setFocus(Qt::MouseFocusReason);
  if(event->buttons() & Qt::LeftButton){
    QPointF mouse_mov = event->scenePos()-event->lastScenePos();  
    QTransform init = this->transform();
    /* Take into account the zoom of the item */
    QLine l = init.map(QLine(1,0,0,0));
    qreal scale = sqrt(l.dx()*l.dx()+l.dy()+l.dy());
    QTransform move = QTransform().translate(mouse_mov.x()/scale, mouse_mov.y()/scale);    
    this->setTransform(init*QTransform().translate(mouse_mov.x(), mouse_mov.y()));
   }
}
void ImageItem::wheelEvent( QGraphicsSceneWheelEvent * event ){
  QPointF scene_pos = (event->scenePos());
  QPointF mouse_pos = mapFromScene(scene_pos);
  qreal speed = 0.005;
  qreal scale = 1+event->delta()*speed;
  QTransform trans = this->transform();
  qDebug("pos %f %f scene pos %f %fscale %f",mouse_pos.x(),mouse_pos.y(),scene_pos.x(),scene_pos.y(),scale);
  this->setTransform(trans.scale(scale, scale).translate(mouse_pos.x(), mouse_pos.y()).translate(-mouse_pos.x()*scale,-mouse_pos.y()*scale));
}

void ImageItem::keyReleaseEvent ( QKeyEvent * event ){
  if(event->matches(QKeySequence::Delete) ||
     event->matches(QKeySequence::Cut) ||
     event->key() & Qt::Key_Backspace){
    // I don't know if this is safe;
    this->scene()->removeItem(this);
    
  }
}
