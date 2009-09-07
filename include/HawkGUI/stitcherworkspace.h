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
  void onArrowClicked();
  void onLineClicked();
  void onCircleClicked();
  void onRotateClicked();
  void onItemChanged(QStandardItem * item);
  void loadGeometry();
  void initConstraintsTree();
  void onAddControlPointClicked();
  void onDeleteControlPointClicked();
  void onAddConstraintClicked();
  void onDelConstraintClicked();
  void onOptimizeGeometryClicked();
  void onSaveGeometryClicked();
  void onLoadGeometryClicked();
 private:
  QTreeView * createGeometryTree();
  QWidget * createConstraintsTree();
  QTreeView * geometryTree;
  QTreeView * constraintsTree;
  QWidget * createToolBar();
  StitcherView * _stitcherView;
};

#endif
