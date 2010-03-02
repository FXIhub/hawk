#include <QApplication>
#include <QPushButton>
#include <qxtrpcpeer.h>
#include "rpctestclient.h"


int main(int argc, char* argv[])
{
  if(argc < 2){
    printf("Usage: rpcclient <server address>\n");
    exit(1);
  }
  QApplication app(argc, argv);
  QPushButton hello("Hello world!");
  hello.resize(100, 30);
  hello.show();
  Client client(&hello,argv[1]);
  return app.exec();
}
