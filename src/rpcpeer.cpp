#ifdef NETWORK_SUPPORT
#include "rpcpeer.h"
#include <QCoreApplication>
#include <QTimer>

RPCPeer::RPCPeer()
{
  QObject::connect(this,SIGNAL(connectedToServer()),this,SLOT(connectionEstablished()));
  QObject::connect(this,SIGNAL(disconnectedFromServer()),this,SLOT(connectionLost()));
  QObject::connect(this,SIGNAL(serverError(QAbstractSocket::SocketError)),this,SLOT(connectionLost()));
  attachSlot(QString("sendOptions(QByteArray)"),this,SLOT(receiveOptions(QByteArray)));
}

void RPCPeer::connect(QHostAddress addr , int port){
  QxtRPCPeer::connect(addr,port);
  m_timer.setSingleShot(true);
  QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkTimeOut()));
  m_serverAddress = addr;
  m_serverPort = port;
  m_timer.start(5000);
}

void RPCPeer::connectionEstablished(){
  qDebug("RPC Peer: Connection establish");  
  m_timer.stop();
  m_timer.disconnect();
}

void RPCPeer::checkTimeOut(){
    qDebug("RPC Peer: time out while attempting connection!");
    QCoreApplication::exit(-1);
}

void RPCPeer::connectionLost(){
  qDebug("RPC Peer: lost connection to server!");
  m_timer.setSingleShot(false);
  QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(attemptReconnection()));
  m_timer.start(5000);

}

void RPCPeer::attemptReconnection(){  
  qDebug("RPC Peer: Attempting reconnection...");
  QxtRPCPeer::connect(m_serverAddress,m_serverPort);
}

void RPCPeer::receiveOptions(QByteArray optionsFile){
  qDebug("RPC Peer: Received Options");
}


#endif
