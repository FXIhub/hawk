#ifndef _IMAGEVIEWPANEL_H_
#define _IMAGEVIEWPANEL_H_
#if defined __cplusplus || defined Q_MOC_RUN

#include <QToolBar>
#include <QWidget>
#include <QFrame>
#include <QTimer>

class ImageView;
class QTimer;
class QComboBox;

class ImageViewPanel: public QWidget
{
  Q_OBJECT
    public:
  ImageViewPanel(ImageView * parent = 0);
  bool eventFilter(QObject * w, QEvent * e);
private slots:
    void changeVisibility();
    //    void setLinearScale();
    //    void setLogScale();
    void changeColormap(int index);
 private:
  QFrame * frame;
  QList<QObject *>underMouse;
  ImageView * imageView;
  QTimer visibilityTimer;
  QComboBox * colormapCombo;

};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
