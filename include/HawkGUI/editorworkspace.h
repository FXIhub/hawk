#ifndef _EDITOR_WORKSPACE_H_
#define _EDITOR_WORKSPACE_H_ 1

#include <QWidget>
#include <QModelIndex>

class QTreeView;
class QTreeWidgetItem;
class ImageEditorView;
class QStandardItem;
class QStackedLayout;
class QGroupBox;

class EditorWorkspace: public QWidget
{
  Q_OBJECT
    public:
  EditorWorkspace(QWidget * parent);
 private:
  QTreeView * createPropertiesTree();
  QWidget * createTools();
  QWidget * dropToolOptions;
  QWidget * filterToolOptions;
  QWidget * selectionToolOptions;
  QTreeView * propertiesTree;
  ImageEditorView * editorView;
  QWidget * toolOptions;
  QStackedLayout * toolOptionsLayout;
  private slots:
  void clicked(QModelIndex index);
  void onItemChanged(QStandardItem * item);
  void loadProperties();
  void onMathEdit();
  void onFilterClicked();
  void onPointerClicked();
  void onBullseyeClicked();
  void onDropClicked();
  void onSelectionClicked();
  void onLineoutClicked();
};
#endif
