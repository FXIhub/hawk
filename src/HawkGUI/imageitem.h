#ifndef _IMAGEITEM_H_
#define _IMAGEITEM_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsPixmapItem>
#include <QObject>
#include "imageview.h"
#include <spimage.h>
#include <QStack>
#include <QRegion>

class QGraphicsSceneMouseEvent;
class ImageView;
class QGraphicsEllipseItem;

class ImageItem: public QObject, public QGraphicsPixmapItem
{
  Q_OBJECT
    Q_PROPERTY(double HawkGeometry_dx READ dx WRITE setDx)
    Q_PROPERTY(double HawkGeometry_dy READ dy WRITE setDy)
    Q_PROPERTY(double HawkGeometry_dz READ dz WRITE setDz)
    Q_PROPERTY(double HawkGeometry_theta READ theta WRITE setTheta)
    Q_PROPERTY(double HawkGeometry_alpha READ alpha WRITE setAlpha)
    Q_PROPERTY(bool HawkGeometry_dx_locked READ dxLocked WRITE setDxLocked)
    Q_PROPERTY(bool HawkGeometry_dy_locked READ dyLocked WRITE setDyLocked)
    Q_PROPERTY(bool HawkGeometry_dz_locked READ dzLocked WRITE setDzLocked)
    Q_PROPERTY(bool HawkGeometry_theta_locked READ thetaLocked WRITE setThetaLocked)
    Q_PROPERTY(bool HawkGeometry_alpha_locked READ alphaLocked WRITE setAlphaLocked)
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

  void setImageMask(QPoint pos, int value);

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
  void interpolateEmpty(double radius,int iterations,QRegion selected,QString kernel);
  void cropImage(QRegion selected);
  void showIdentifier(bool show = true);
  double dx() const;
  void setDx(double);
  double dy() const;
  void setDy(double);
  double dz() const;
  void setDz(double);
  double theta() const;
  void setTheta(double);
  double alpha() const;
  void setAlpha(double);
  bool dxLocked() const;
  void setDxLocked(bool);
  bool dyLocked() const;
  void setDyLocked(bool);
  bool dzLocked() const;
  void setDzLocked(bool);
  bool thetaLocked() const;
  void setThetaLocked(bool);
  bool alphaLocked() const;
  void setAlphaLocked(bool);
  QString identifier() const;
  void addControlPoint(QPointF pos);
  void deleteControlPoint(QPointF pos);
  QList<QPointF> getControlPoints();
  void moveBy(qreal dx, qreal dy);
  QTransform transformFromParameters();
  QList<QPoint> imagePointsAround(QPointF scenePos,int sceneRadius);
  void setMaskFromImage(const Image * mask);
  void invertMask();
 private:
  enum EditType{ImageSize,Phased,Shifted,Wavelength,DetectorDistance,PointConvolute,Scaled,PixelSize,ImageCenter,CheckPoint,ImageMask,MaskFromImage,InvertMask};
  struct EditStep{
    EditType type;
    QVector<QVariant> arguments;
  };
  void addToStack(EditType type,QVariant arg1 = QVariant(),QVariant arg2 = QVariant());
  void applyEditStep(EditStep step);
  void repositionCenterIndicators();
  double overallScale() const;
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
  QString identifierString;
  QList<QGraphicsEllipseItem *> controlPoints;
  bool _dxLocked;
  bool _dyLocked;
  bool _dzLocked;
  bool _thetaLocked;
  qreal _dx;
  qreal _dy;
  qreal _dz;
  qreal _theta;
  qreal _alpha;
  bool _alphaLocked;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
