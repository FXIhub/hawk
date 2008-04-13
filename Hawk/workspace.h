#ifndef _WORKSPACE_H_
#define _WORKSPACE_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "ui_workspace.h"
#include <spimage.h>

class MainWindow;

class Workspace: public QWidget , private Ui::Workspace
{
  Q_OBJECT
    public:
  Workspace(QWidget * parent = NULL,MainWindow * main=NULL);
  QTableWidget * getPropertiesTable();
 public slots:
  void loadImage(QString filename);
 private:
  void setupViewers();
  MainWindow * mainWindow;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
