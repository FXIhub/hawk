#include <QApplication>
#include "preview.h"

int main(int argc, char *argv[])
{

  
  QApplication app(argc, argv);
  Preview preview;
  preview.show();
  return app.exec();
}
