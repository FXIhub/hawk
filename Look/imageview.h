#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QtGui>
#include <spimage.h>

class ImageView : public QWidget
{
  Q_OBJECT
    public:
  ImageView(QWidget *parent = 0);
  void setImage(QImage *image);
  bool collumnToMask();
  void setCenter(int on);
  bool setCenterActivated();
  void setCenter(real x, real y);
  void getCenter(real *x, real *y);
  void defineBeamstop(int on);
  bool defineBeamstopActivated();
  void setBeamstop(real x, real y, real r);
  void getBeamstop(real *x, real *y, real *r);
  void showDistance(int on, bool bar, real value);
  QPoint pickSpot();
    
 protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);

 signals:
  void centerChanged();
  void beamstopChanged();
  void vertLineSet();

 private:
  void drawImage();
  int globalToScreenX(real x);
  int globalToScreenY(real y);
  real screenToGlobalX(int x);
  real screenToGlobalY(int y);

  QLabel *imageLabel;
  QImage *originalQi;
  QImage imageQi;

  real zoomValue;
  real startZoomValue;
  real zoomCenterX, zoomCenterY;
  real startZoomCenterX, startZoomCenterY;

  bool leftPressed, rightPressed;

  bool paneActive;
  bool zoomActive;
  QPoint startPos;

  bool setCenterActive;
  real centerX, centerY;
  bool moveCenterX, moveCenterY;
  
  bool defineBeamstopActive;
  real beamstopX, beamstopY, beamstopR;
  real beamstopOffsetX, beamstopOffsetY, beamstopStartR, beamstopOriginalR;
  bool beamstopMove, beamstopMoveR;

  real distanceBarLength;
  QString distanceBarUnit;
  real circleResolution;

  bool pickSpotActive;
};

#endif
