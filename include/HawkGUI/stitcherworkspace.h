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
  void onDeleteGuideClicked();
  void onAddConstraintClicked();
  void onAddConstraintDialogClosed();
  void onDelConstraintClicked();
  void onOptimizeGeometryClicked();
  void onSaveGeometryClicked();
  void onSaveControlPointsClicked();
  void onLoadGeometryClicked();
  void onLoadControlPointsClicked();
  void clearAll();
 private:
  QTreeView * createGeometryTree();
  QWidget * createConstraintsTree();
  QTreeView * geometryTree;
  QTreeView * constraintsTree;
  QWidget * createToolBar();
  StitcherView * _stitcherView;
};

void transformShow(QTransform t);

#endif
