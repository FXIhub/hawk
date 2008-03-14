#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QMainWindow>
#include "ui_Hawk.h"
#include "workspace.h"

class Server;

class MainWindow: public QMainWindow, private Ui::Hawk
{
  Q_OBJECT
    public:
  MainWindow(QMainWindow * parent = NULL);
 private:
  Server * server;
  QList<Workspace *> workspaces;
 private slots:
  void on_actionOpen_triggered(bool checked);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
