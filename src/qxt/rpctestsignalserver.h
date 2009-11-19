#ifndef _RPCSERVER_H_
#define _RPCSERVER_H_ 1

#include <qxtrpcpeer.h>
#include <QPushButton>
#include <QMessageBox>

class Server : public QObject {
Q_OBJECT
public:
	Server() {
		m_rpc = new QxtRPCPeer();
		m_rpc->listen(QHostAddress::Any, 45600);
		m_rpc->attachSlot(QString("exiting()"),this,SLOT(clientExiting(quint64)));
	}

	private slots:
	void clientExiting(quint64 id){
	  QMessageBox::information(0, tr("test"),
				   "Client "+QString::number(id) +" exiting",
			    QMessageBox::Ok,QMessageBox::Ok);
	}
private:
	QxtRPCPeer* m_rpc;
};

#endif
