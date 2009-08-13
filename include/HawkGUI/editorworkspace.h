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
 public:
  ImageEditorView * editorView() const;
 private:
  QTreeView * createPropertiesTree();
  QWidget * createTools();
  QTreeView * propertiesTree;
  ImageEditorView * _editorView;
  private slots:
  void clicked(QModelIndex index);
  void onItemChanged(QStandardItem * item);
  void loadProperties();
};
#endif
