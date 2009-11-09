#include <QCoreApplication>
#include <qxtrpcpeer.h>

class Server {
public:
	Server() {
		m_rpc = new QxtRPCPeer();
		m_rpc->listen(QHostAddress::Any, 45600);
	}

private:
	QxtRPCPeer* m_rpc;
};

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	Server server;
	return app.exec();
}

