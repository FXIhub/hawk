#ifndef _IMAGEITEM_H_
#define _IMAGEITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPixmapItem>
#include "imageview.h"
#include <spimage.h>
#include <QStack>
#include <QRegion>

class QGraphicsSceneMouseEvent;
class ImageView;

class ImageItem: public QGraphicsPixmapItem
{
   public:
  ImageItem(Image * data,QString filename,ImageView * view,QGraphicsItem * parent = NULL);
  ImageItem(QPixmap pix,ImageView * view,QGraphicsItem * parent);
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
  const Image * getImage();
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
  void updateImage();
  void setImageCenter(QPointF center);
  QPointF imageCenter() const;
  void setPixelSize(QSizeF size);
  QSizeF pixelSize() const;
  void pointConvolute(QPointF scenePos, const Image * kernel);
  void setDetectorDistance(double distance);
  double detectorDistance()const;
  void setWavelength(double distance);
  double wavelength()const;
  bool shifted() const;
  void setShifted(bool shifted);
  bool scaled() const;
  void setScaled(bool scaled);
  bool phased() const;
  void setPhased(bool phased);
  QSize imageSize()const;
  void setImageSize(QSize size);
  void undoEditSteps(int numSteps = 1);
  void redoEditSteps(int numSteps = 1);
  void removeVerticalLines(QRect rect);
  void removeHorizontalLines(QRect rect);
  void setCenterIndicatorsVisible(bool show = true);
  void setSelected(bool selelected = true);
  void rotateImage();
  void xcamPreprocess();
  void interpolateEmpty(double radius,int iterations,QRegion selected);
  void cropImage(QRegion selected);
  void showIdentifier(bool show = true);
 private:
  enum EditType{ImageSize,Phased,Shifted,Wavelength,DetectorDistance,PointConvolute,Scaled,PixelSize,ImageCenter,CheckPoint};
  struct EditStep{
    EditType type;
    QVector<QVariant> arguments;
  };
  void addToStack(EditType type,QVariant arg1,QVariant arg2 = QVariant());
  void applyEditStep(EditStep step);
  void repositionCenterIndicators();

  QGraphicsRectItem * selectRect;
  QString filename;
  Image * image;
  uchar * colormap_data;
  int colormap_flags;
  real colormap_min;
  real colormap_max;
  QImage data;
  bool selected;
  QStack<EditStep> undoStack;
  QStack<EditStep> redoStack;
  ImageView * _view;
  QGraphicsLineItem * centerVerticalIndicator;
  QGraphicsLineItem * centerHorizontalIndicator;
  QGraphicsTextItem * identifierItem;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
