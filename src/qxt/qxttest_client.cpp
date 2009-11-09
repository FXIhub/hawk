#include <QCoreApplication>
#include <qxtrpcpeer.h>

class Client {
public:
	Client() {
		m_rpc = new QxtRPCPeer();
		m_rpc->connect(QHostAddress::LocalHost, 45600);
	}

private:
	QxtRPCPeer* m_rpc;
};

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	Client client;
	return app.exec();
}
