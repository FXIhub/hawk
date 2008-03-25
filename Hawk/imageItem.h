#ifndef _IMAGEITEM_H_
#define _IMAGEITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

class ImageItem: public QGraphicsPixmapItem
{
 public:
  ImageItem(QGraphicsItem * parent = NULL);
  ImageItem(const QPixmap & pix, QGraphicsItem * parent = NULL );  
  void mouseMove(QGraphicsSceneMouseEvent * event){
    mouseMoveEvent(event);
  }
  void wheel( QGraphicsSceneWheelEvent * event ){
    wheelEvent ( event);
  }
  void select();
  void deselect();
  QPointF centeredScale(qreal scale,QPointF screenCenter);
  void focusInEvent ( QFocusEvent * event );
  void focusOutEvent ( QFocusEvent * event );
 private:
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void wheelEvent ( QGraphicsSceneWheelEvent * event );
  void keyReleaseEvent ( QKeyEvent * event );
  QGraphicsRectItem * selectRect;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
