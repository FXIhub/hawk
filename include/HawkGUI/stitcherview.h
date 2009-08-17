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
    enum Mode{Default,Line,Circle};
    void setMode(Mode m);
    ImageItem * selected();
 public slots:
    void clearHelpers();
 protected:
  void mouseReleaseEvent(QMouseEvent * e);
  void mousePressEvent(QMouseEvent * e);
  void mouseMoveEvent(QMouseEvent * e);
  void paintEvent(QPaintEvent * e);
 private:
  ImageItem * selectedItem;
  Mode mode;
  QPoint lineOrigin;
  QPoint lineEnd;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif

