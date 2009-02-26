#ifndef _IMAGEITEM_H_
#define _IMAGEITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPixmapItem>
#include "spimage.h"
#include "applicationmode.h"

class QGraphicsSceneMouseEvent;
class MainWindow;

class ImageItem: public QGraphicsPixmapItem
{
   public:
  ImageItem(Image * data,MainWindow * main,QString filename,QGraphicsItem * parent = NULL);
  ~ImageItem();
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
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
	     QWidget * widget);
  void setMode(ApplicationMode newMode);
  void excludeFromMask(QRectF area);
  void includeInMask(QRectF area);
  Image * getImage();
  QString getFilename(){
    return filename;
  }
  void setImageCenter(QPointF scenePos);
  bool isSelected();
 private:
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void wheelEvent ( QGraphicsSceneWheelEvent * event );
  void keyReleaseEvent ( QKeyEvent * event );
  QGraphicsRectItem * selectRect;
  QString filename;
  Image * image;
  uchar * colormap_data;
  int colormap_flags;
  real colormap_min;
  real colormap_max;
  MainWindow * mainWindow;
  ApplicationMode mode;
  QImage data;
  QImage mask;
  QImage maskFaint;
  QImage sniperScope;
  bool selected;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
