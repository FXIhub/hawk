#ifndef _EDITOR_WORKSPACE_H_
#define _EDITOR_WORKSPACE_H_ 1

#include <QWidget>
#include <QModelIndex>

class QTreeView;
class QTreeWidgetItem;

class EditorWorkspace: public QWidget
{
  Q_OBJECT
    public:
  EditorWorkspace(QWidget * parent);
 private:
  QTreeView * createPropertiesTree();
  QTreeView * propertiesTree;
  private slots:
  void clicked(QModelIndex index);
};
#endif
