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
class EditorTools;
class EditorWorkspace: public QWidget
{
  Q_OBJECT
    public:
  EditorWorkspace(QWidget * parent);
 public:
  ImageEditorView * editorView() const;
  EditorTools * editorTools() const;
 private:
  QTreeView * createPropertiesTree();
  QWidget * createTools();
  QTreeView * propertiesTree;
  ImageEditorView * _editorView;
  EditorTools * _editorTools;
  private slots:
  void clicked(QModelIndex index);
  void onItemChanged(QStandardItem * item);
  void loadProperties();
};
#endif
