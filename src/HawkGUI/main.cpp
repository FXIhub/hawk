#include <QtGui>
#include <QStringList>
#include "hawkgui.h"
#include "configuration.h"
#include "uwrapc.h"
#include "rpcdefaultport.h"
#include "processcontrol.h"

void initSettings(){
  QSettings settings;
  QStringList profiles = settings.value("RemoteLaunchDialog/profileList").toStringList();
  if(profiles.isEmpty()){
    /*
      There are no stored settings. 
      Set some defaults.
    */
    profiles << QString("default");
    settings.setValue("RemoteLaunchDialog/profileList",profiles);
    settings.setValue("RemoteLaunchDialog/default/remoteHost",QString("localhost"));
    settings.setValue("RemoteLaunchDialog/default/remotePort",22);
    settings.setValue("RemoteLaunchDialog/default/localHost",QString("localhost"));
    settings.setValue("RemoteLaunchDialog/default/autoLocalPort",true);
    settings.setValue("RemoteLaunchDialog/default/localPort",rpcDefaultPort);
    settings.setValue("RemoteLaunchDialog/default/sshPath","/usr/bin/ssh");
    settings.setValue("RemoteLaunchDialog/default/uwrapcPath","/usr/bin/uwrapc");
    settings.setValue("RemoteLaunchDialog/selectedProfile",QString("default"));
  }
  bool ok = false;
  settings.value("ProcessControl/launchMethod").toInt(&ok);
  if(!ok){
    settings.setValue("ProcessControl/launchMethod",ProcessControl::LaunchLocally);
  }
}


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

  QCoreApplication::setOrganizationName("Hawk");
  QCoreApplication::setOrganizationDomain("xray.bmc.uu.se");
  QCoreApplication::setApplicationName("HawkGUI");
  initSettings();

  HawkGUI hawkgui;
  hawkgui.show();
    
  return app.exec();
}
