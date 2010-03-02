#include <QApplication>
#include <QPushButton>
#include <qxtrpcpeer.h>
#include "rpctestsignalclient.h"


int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  QPushButton hello("Exit!");
  hello.resize(100, 30);
  hello.show();
  Client client(&hello);
  return app.exec();
}
