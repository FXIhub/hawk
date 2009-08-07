#ifndef _EDITOR_WORKSPACE_H_
#define _EDITOR_WORKSPACE_H_ 1

#include <QWidget>
#include <QModelIndex>

class QTreeView;
class QTreeWidgetItem;
class ImageEditorView;
class QStandardItem;


class EditorWorkspace: public QWidget
{
  Q_OBJECT
    public:
  EditorWorkspace(QWidget * parent);
 private:
  QTreeView * createPropertiesTree();
  QTreeView * propertiesTree;
  ImageEditorView * editorView;
  private slots:
  void clicked(QModelIndex index);
  void onItemChanged(QStandardItem * item);
  void loadProperties();
};
#endif
