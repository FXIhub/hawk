#ifndef OPTIONSTREE_H
#define OPTIONSTREE_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "configuration.h"


class ComboBoxDelegate;


class OptionsTree : public QWidget
{
    Q_OBJECT

public:
    OptionsTree(QWidget * parent = NULL);
    ~OptionsTree();
 public slots:
    void showAdvancedOptions(bool show);
    void rebuildTree();
    void resetSelectedOption();
    void resetAllOptions();
    void saveOptions();
    void loadOptions();
 signals:
    void optionsTreeUpdated(Options * opt);
private:
    void createGUI();
    void setDefaultOptions(Options * opt);    
    QList<QTreeWidgetItem *>allOptions;
    QList<QTreeWidgetItem *>defaultOptions;
    QList<QTreeWidgetItem *>advancedOptions;
    bool showAdvancedOptionsToggled;
    QTreeWidget * tree;
    ComboBoxDelegate * delegate;
};

#endif
