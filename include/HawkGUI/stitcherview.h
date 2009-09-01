#ifndef _STITCHERVIEW_H_
#define _STITCHERVIEW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "imageview.h"
class ImageItem;

class StitcherView: public ImageView
{
  Q_OBJECT
    public:
  StitcherView(QWidget * parent);
  void addImage(ImageItem * item);
  bool loadImage(QString file);
  void saveImage();
  enum Mode{Default,Line,Circle,AddPoint,DeletePoint};
  void setMode(Mode m);
 public slots:
  void clearHelpers();
  void clearAll();
  void scaleItems(qreal new_scale);
  void scaleScene(qreal new_scale);
 protected:
  void mouseReleaseEvent(QMouseEvent * e);
  void mousePressEvent(QMouseEvent * e);
  void mouseMoveEvent(QMouseEvent * e);
  void paintEvent(QPaintEvent * e);
  void keyPressEvent ( QKeyEvent * event );
 private:
  Mode mode;
  QPoint lineOrigin;
  QPoint lineEnd;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif

