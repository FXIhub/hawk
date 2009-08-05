#ifndef _EDITOR_WORKSPACE_H_
#define _EDITOR_WORKSPACE_H_ 1

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

class EditorWorkspace: public QWidget
{
  Q_OBJECT
    public:
  EditorWorkspace(QWidget * parent);
 private:
  QTreeWidget * createPropertiesTree();
  QTreeWidget * propertiesTree;
  private slots:
};
#endif
