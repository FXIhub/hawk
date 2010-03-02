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
  Client(QPushButton * button,QString serverAddress) {
		m_rpc = new QxtRPCPeer();
		QHostInfo serverInfo = QHostInfo::fromName(serverAddress);
		m_rpc->connect(serverInfo.addresses().first(), 45600);
		m_rpc->attachSlot(QString("released()"),button,SLOT(toggle()));
		m_rpc->attachSlot(QString("pressed()"),button,SLOT(toggle()));
		button->setCheckable(true);
		connect(button,SIGNAL(clicked()),this,SLOT(sendData()));
		m_rpc->attachSignal(this,SIGNAL(dataSent(QByteArray)));
	}

	signals:
	void dataSent(QByteArray data);
private slots:
	void sendData(){
	   QByteArray byteArray;
	   QDataStream out(&byteArray,QIODevice::WriteOnly);
	   //	   out << "foo goo foo";
	   //	   out << QApplication::palette();
	   QString s = "foo goo foo";
	   out << s;
	   qDebug("Sending %s",s.toAscii().data());
	   emit dataSent(byteArray);
	}
private:	
	QxtRPCPeer* m_rpc;
};

#endif
