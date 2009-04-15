#include <QtGui>
#include <QStringList>
#include "hawkgui.h"
#include "configuration.h"
#include "uwrapc.h"

void uwrapc_main(){
  FILE * f;
  Options * opts = &global_options;
  void * socket = 0;
  set_defaults(opts);

  f = fopen("uwrapc.conf","rb");
  if(f){
    fclose(f);
    read_options_file("uwrapc.conf");
    check_options_and_load_images(opts);
    write_options_file("uwrapc.confout");
  }else if(!socket){
    perror("Could not open uwrapc.conf");
  }
  srand(get_random_seed(opts));
  
  init_reconstruction(opts);
  /* cleanup stuff */
  if(opts->init_support){
    sp_image_free(opts->init_support);
  }
  if(opts->diffraction){
    sp_image_free(opts->diffraction);
  }
  if(opts->amplitudes){
    sp_image_free(opts->amplitudes);
  }
  if(opts->intensities_std_dev){
    sp_image_free(opts->intensities_std_dev);
  }
}

int main(int argc, char **argv)
{
  if(argc == 2 && strcmp(argv[1] ,"uwrapc") == 0){
    uwrapc_main();
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
