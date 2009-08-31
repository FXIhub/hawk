#ifndef _STITCHER_WORKSPACE_H_
#define _STITCHER_WORKSPACE_H_ 1

#include <QWidget>
class StitcherView;
class QMouseEvent;
class ImageItem;
class QTreeView;
class QStandardItem;

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
  void onItemChanged(QStandardItem * item);
  void loadGeometry();
 private:
  QTreeView * createGeometryTree();
  QTreeView * geometryTree;
  QWidget * createToolBar();
  StitcherView * _stitcherView;
};

#endif
