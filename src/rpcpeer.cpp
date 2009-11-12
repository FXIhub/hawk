#ifdef NETWORK_SUPPORT
#include "rpcpeer.h"
#include <QCoreApplication>
#include <QTimer>
#include <QTemporaryFile>
#include "configuration.h"
#include "uwrapcpeerthread.h"

RPCPeer::RPCPeer(RPCInfo * rpcInfo)
  :m_rpcInfo(rpcInfo)
{
  QObject::connect(this,SIGNAL(connectedToServer()),this,SLOT(connectionEstablished()));
  QObject::connect(this,SIGNAL(disconnectedFromServer()),this,SLOT(connectionLost()));
  attachSlot(QString("sendOptions(QByteArray)"),this,SLOT(receiveOptions(QByteArray)));
  attachSlot(QString("startReconstruction()"),this,SLOT(startReconstruction()));
}

void RPCPeer::connect(QHostAddress addr , int port){
  QxtRPCPeer::connect(addr,port);
  m_timer.setSingleShot(true);
  QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkTimeOut()));
  m_timer.start(5000);
}

void RPCPeer::connectionEstablished(){
  qDebug("RPC Peer: Connection established");  
  m_timer.stop();
  m_timer.disconnect();
  QObject::connect(this,SIGNAL(serverError(QAbstractSocket::SocketError)),this,SLOT(connectionLost()));
  /* new connectedToServer will mean the connection was recovered not established */
  QObject::disconnect(this,SIGNAL(connectedToServer()),this,SLOT(connectionEstablished()));
  QObject::connect(this,SIGNAL(connectedToServer()),this,SLOT(connectionRecovered()));
}

void RPCPeer::connectionRecovered(){
  qDebug("RPC Peer: Connection recovered");  
  m_timer.stop();
  m_timer.disconnect();
  QObject::connect(this,SIGNAL(serverError(QAbstractSocket::SocketError)),this,SLOT(connectionLost()));
}

void RPCPeer::checkTimeOut(){
    qDebug("RPC Peer: time out while attempting connection!");
    QCoreApplication::exit(-1);
}

void RPCPeer::connectionLost(){
  /* Only start the timer on the first failure */
  if(!m_timer.isActive()){
    qDebug("RPC Peer: lost connection to server!");
    m_timer.setSingleShot(false);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(attemptReconnection()));
    m_timer.start(5000);
  }
}

void RPCPeer::attemptReconnection(){  
  qDebug("RPC Peer: Attempting reconnection...");
  QxtRPCPeer::connect(m_rpcInfo->serverInfo.addresses().first(),m_rpcInfo->serverPort);
}

void RPCPeer::receiveOptions(QByteArray optionsString){
  qDebug("RPC Peer: Received Optionsy:\n\n%s",optionsString.constData());
  QTemporaryFile file;
  if (file.open()) {
    if(file.write(optionsString) != optionsString.size()){
      qDebug("RPCPeer: failed to write temporary file!");      
    }
    if(!file.flush()){
      qDebug("RPCPeer: failed to flush temporary file!");
    }
    read_options_file(file.fileName().toAscii().constData());
  }else{
    qDebug("RPCPeer: failed to create temporary file!");
    return;
  }
}

void RPCPeer::startReconstruction(){
  qDebug("RPCPeer: Reconstruction started");
  m_thread = new UwrapcPeerThread(m_rpcInfo);
  QObject::connect(m_thread,SIGNAL(finished()),this,SLOT(threadFinished()));
  QObject::connect(m_thread,SIGNAL(reconstructionStarted()),this,SLOT(reconstructionStarted()));
  attachSignal(m_thread,SIGNAL(reconstructionStarted()),QString("reconstructionStarted()"));
  m_thread->start();
}

void RPCPeer::threadFinished(){
  qDebug("RPCPeer: Reconstruction thread exited.");
  qDebug("RPCPeer: Thread id %p",(void *)QThread::currentThread());
}

void RPCPeer::reconstructionStarted(){
  qDebug("RPCPeer: Reconstruction started.");
  qDebug("RPCPeer: Thread id %p",(void *)QThread::currentThread());
}
#endif
