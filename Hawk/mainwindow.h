#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QMainWindow>

class Server;

class MainWindow: public QMainWindow
{
  Q_OBJECT
    public:
  MainWindow();
 private:
  Server * server;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
