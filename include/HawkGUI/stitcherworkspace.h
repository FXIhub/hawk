#ifndef _STITCHER_WORKSPACE_H_
#define _STITCHER_WORKSPACE_H_ 1

#include <QWidget>
class StitcherView;
class QMouseEvent;
class ImageItem;
class StitcherWorkspace: public QWidget
{
  Q_OBJECT
    public:
  StitcherWorkspace(QWidget * parent);
 private slots:
  void onStitchClicked();
  void onLineClicked();
  void onCircleClicked();
  void onRotateClicked();
 private:
  QWidget * createToolBar();
  StitcherView * _stitcherView;
};

#endif
