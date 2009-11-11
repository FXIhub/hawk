#ifndef _RPCSERVER_H_
#define _RPCSERVER_H_ 1

#include <qxtrpcpeer.h>
#include <QPushButton>

class Server : public QObject {
Q_OBJECT
public:
	Server(QPushButton * button) {
		m_rpc = new QxtRPCPeer();
		m_rpc->listen(QHostAddress::Any, 45600);
		m_rpc->attachSignal(button,SIGNAL(released()));
		m_rpc->attachSignal(button,SIGNAL(pressed()));
		m_rpc->attachSlot(QString("dataSent(QByteArray)"),this,SLOT(receiveData(quint64,QByteArray)));
	}

	private slots:
	void receiveData(quint64 id, QByteArray data){
	  qDebug("here");
	  QDataStream out(&data,QIODevice::ReadOnly);
	  QString s;
	  out >> s;
	  qDebug("received %s from %llu",s.toAscii().data(),id);
	}

private:
	QxtRPCPeer* m_rpc;
};

#endif
