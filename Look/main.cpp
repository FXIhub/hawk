#include <QtGui>
//#include <QApplication>
//#include <spimage.h>

//#include "mainwindow.moc"
#include "mainwindow.h"

int main(int argc, char ** argv)
{
  QApplication app(argc, argv);
  MainWindow mainWindow;
  mainWindow.setGeometry(100, 100, 800, 600);
  mainWindow.show();
  return app.exec();
}
