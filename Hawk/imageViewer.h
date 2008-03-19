#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsScene>
#include <QGraphicsView>
#include "imageItem.h"

class ImageViewer: public QGraphicsScene
{
  Q_OBJECT
    public:
  ImageViewer(QGraphicsView * view, QWidget * parent = NULL);
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void wheelEvent ( QGraphicsSceneWheelEvent * event );
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent );
  void addImage(ImageItem * item);
  void keyReleaseEvent ( QKeyEvent * event );
  void scaleItems(qreal scale);
 private:
  QGraphicsView * graphicsView;
  QGraphicsItem * dragged;
  QPointF draggedInitialPos;
  QPointF itemsScale;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
