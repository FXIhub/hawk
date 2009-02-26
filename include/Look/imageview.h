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
  void setCenter(double x, double y);
  void getCenter(double *x, double *y);
  void defineBeamstop(int on);
  bool defineBeamstopActivated();
  void setBeamstop(double x, double y, double r);
  void getBeamstop(double *x, double *y, double *r);
  void showDistance(int on, bool bar, double value);
  void getVertLine();
  void drawMask(bool on);
  void undrawMask(bool on);
    
 protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void leaveEvent(QEvent * event);

 signals:
  void centerChanged();
  void beamstopChanged();
  void vertLineSet(double x, double y);
  void drawMaskAt(double x, double y);
  void undrawMaskAt(double x, double y);
  void mouseOverImage(double frac_x, double frac_y);
  void mouseLeftImage();

 private:
  void drawImage();
  int globalToScreenX(double x);
  int globalToScreenY(double y);
  double screenToGlobalX(int x);
  double screenToGlobalY(int y);

  QLabel *imageLabel;
  QImage *originalQi;
  QImage imageQi;

  double zoomValue;
  double startZoomValue;
  double zoomCenterX, zoomCenterY;
  double startZoomCenterX, startZoomCenterY;

  QCursor cursorCross;

  bool leftPressed, rightPressed;

  bool paneActive;
  bool zoomActive;
  QPoint startPos;

  bool setCenterActive;
  double centerX, centerY;
  bool moveCenterX, moveCenterY;
  
  bool defineBeamstopActive;
  double beamstopX, beamstopY, beamstopR;
  double beamstopOffsetX, beamstopOffsetY, beamstopStartR, beamstopOriginalR;
  bool beamstopMove, beamstopMoveR;

  double distanceBarLength;
  QString distanceBarUnit;
  double circleResolution;

  bool pickSpotActive;
  bool drawMaskActive, undrawMaskActive;
};

#endif
