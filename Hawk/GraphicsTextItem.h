#ifndef _GRAPHICSTEXTITEM_H_
#define _GRAPHICSTEXTITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsTextItem>

class GraphicsTextItem: public QGraphicsTextItem
{
 public:
  void mousePressEvent(QGraphicsSceneMouseEvent * event );
 private: 
  QGraphicsTextItem * label;
};

#endif
#endif
