#ifndef _IMAGEITEM_H_
#define _IMAGEITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPixmapItem>
#include "imageview.h"
#include "spimage.h"

class QGraphicsSceneMouseEvent;

class ImageItem: public QGraphicsPixmapItem
{
   public:
  ImageItem(Image * data,QString filename,QGraphicsItem * parent = NULL);
  ImageItem(QPixmap pix,QGraphicsItem * parent);
  ~ImageItem();

  enum { Type = UserType + 1 };
  void mouseMove(QGraphicsSceneMouseEvent * event){
    mouseMoveEvent(event);
  }
  void wheel( QGraphicsSceneWheelEvent * event ){
    wheelEvent ( event);
  }
  QPointF centeredScale(qreal scale,QPointF screenCenter);
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
	     QWidget * widget);
  Image * getImage();
  QString getFilename(){
    return filename;
  }
  bool isSelected();
  QPointF getScale();
  int type() const{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
  }
  void shiftImage();
  void fourierTransform(QRectF area,bool squared);
  void setColormap(int color);
  int colormap();
  void setDisplay(int display);
  int display();
  void maxContrast(QRectF area);
  void setLogScale(bool on);
  bool logScale();
  bool isShifted();
 private:
  void updateImage();
  QGraphicsRectItem * selectRect;
  QString filename;
  Image * image;
  uchar * colormap_data;
  int colormap_flags;
  real colormap_min;
  real colormap_max;
  QImage data;
  bool selected;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
