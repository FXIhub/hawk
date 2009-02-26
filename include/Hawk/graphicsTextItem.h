#ifndef _GRAPHICSTEXTITEM_H_
#define _GRAPHICSTEXTITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsTextItem>

class GraphicsTextItem: public QGraphicsTextItem
{
 public:
  GraphicsTextItem(QGraphicsItem * parent = NULL)
    :QGraphicsTextItem(parent)
    {
    }
  void mousePressEvent(QGraphicsSceneMouseEvent * event ){
    qDebug("Pressed");
  }
 private: 
  QGraphicsTextItem * label;
};

#endif
#endif
