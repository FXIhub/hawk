#include <QApplication>
#include <QPushButton>
#include <qxtrpcpeer.h>
#include "rpcclient.h"


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QPushButton hello("Hello world!");
	hello.resize(100, 30);
	hello.show();
	Client client(&hello);
	return app.exec();
}
