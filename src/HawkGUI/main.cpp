#include <QtGui>
#include <QStringList>
#include "hawkgui.h"
#include "configuration.h"
#include "uwrapc.h"

int main(int argc, char **argv)
{
  if(argc >= 2 && strcmp(argv[1] ,"uwrapc") == 0){
    uwrapc_main(argc-1,argv+1);
    return 0;
  }

  QApplication app(argc, argv);
  QStringList libPaths = app.libraryPaths();
  QDir dir(QApplication::applicationDirPath());
  dir.cdUp();
  dir.cd("lib");
  libPaths.prepend(dir.absolutePath());
  // don't go around loading plugins i don't want
  QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
  //QApplication::setLibraryPaths(libPaths);
  //  for(int i = 0;i<libPaths.size();i++){
  //qDebug("%s",libPaths.at(i).toAscii().constData());
  //  }
  HawkGUI hawkgui;
  hawkgui.show();
    
  return app.exec();
}
