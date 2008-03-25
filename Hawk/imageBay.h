#ifndef _IMAGEBAY_H_
#define _IMAGEBAY_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

class ImageBay: public QGraphicsPathItem
{
 public:
  ImageBay(QGraphicsItem * parent = NULL);
  ImageBay(Qt::DockWidgetArea position, QString name = QString(), QPen pen = QPen(),QGraphicsItem * parent = NULL);
};

#endif
#endif
