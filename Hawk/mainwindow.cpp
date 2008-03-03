#include "mainwindow.h"
#include "server.h"

MainWindow::MainWindow()
  :QMainWindow()
{
  server = new Server(this,1050);
}
