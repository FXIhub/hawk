#ifndef _RPCCLIENT_H_
#define _RPCCLIENT_H_ 1

#include <qxtrpcpeer.h>
#include <QPushButton>
#include <QBuffer>
#include <QHostInfo>
#include <QApplication>

class Client: public QObject {
Q_OBJECT
public:
  Client(QPushButton * button){
		m_rpc = new QxtRPCPeer();
		m_rpc->connect(QHostAddress::LocalHost, 45600);
		connect(button,SIGNAL(clicked()),this,SLOT(sendExiting()));
		m_rpc->attachSignal(this,SIGNAL(exiting()),QString("exiting()"));
	}
	signals:
	void exiting();
	private slots:
	void sendExiting(){
	  emit exiting();
	  /* Process events to make sure the signal is actually sent */
	  QCoreApplication::processEvents();
	  QCoreApplication::quit();
	}
private:	
	QxtRPCPeer* m_rpc;
};

#endif
