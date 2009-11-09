#include <QApplication>
#include "rpcserver.h"


int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QPushButton hello("Hello world server!");
	hello.resize(100, 30);
	hello.show();
	Server server(&hello);
	return app.exec();
}

