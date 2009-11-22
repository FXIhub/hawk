#ifdef NETWORK_SUPPORT
#include "rpcpeer.h"
#include <QCoreApplication>
#include <QTimer>
#include <QTemporaryFile>
#include "configuration.h"
#include "uwrapcpeerthread.h"

RPCPeer::RPCPeer(RPCInfo * rpcInfo)
  :m_rpcInfo(rpcInfo),m_connected(false)
{
  QObject::connect(this,SIGNAL(connectedToServer()),this,SLOT(connectionEstablished()));
  QObject::connect(this,SIGNAL(disconnectedFromServer()),this,SLOT(connectionLost()));
  attachSlot(QString("sendOptions(QByteArray)"),this,SLOT(receiveOptions(QByteArray)));
  attachSlot(QString("startReconstruction()"),this,SLOT(startReconstruction()));
  attachSlot(QString("stopReconstruction()"),this,SLOT(stopReconstruction()));
  attachSlot(QString("loadImage(QString)"),this,SLOT(loadImage(QString)));
  attachSlot(QString("quit()"),this,SLOT(quit()));
  attachSignal(this,SIGNAL(reconstructionStopped()),QString("reconstructionStopped()"));  
  attachSignal(this,SIGNAL(identificationKeySent(int)),QString("identificationKeySent(int)"));
  attachSignal(this,SIGNAL(messageSent(int,QString)),QString("messageSent(int,QString)"));
  attachSignal(this,SIGNAL(logLineSent(QString)),QString("logLineSent(QString)"));
  attachSignal(this,SIGNAL(imageOutputNotificationSent(QString)),QString("imageOutputNotificationSent(QString)"));

}

void RPCPeer::connect(QString addr , int port){
  qDebug("RPCPeer: connecting to %s:%d",addr.toAscii().constData(),port);
  QxtRPCPeer::connect(addr,port);
  m_timer.setSingleShot(true);
  QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkTimeOut()));
  m_timer.start(10000);
}

void RPCPeer::connectionEstablished(){
  qDebug("RPC Peer: Connection established");  
  m_connected = true;
  m_timer.stop();
  m_timer.disconnect();
  QObject::connect(this,SIGNAL(serverError(QAbstractSocket::SocketError)),this,SLOT(connectionLost()));
  /* new connectedToServer will mean the connection was recovered not established */
  QObject::disconnect(this,SIGNAL(connectedToServer()),this,SLOT(connectionEstablished()));
  QObject::connect(this,SIGNAL(connectedToServer()),this,SLOT(connectionRecovered()));
  emit identificationKeySent(m_rpcInfo->key);
}

void RPCPeer::connectionRecovered(){
  qDebug("RPC Peer: Connection recovered");  
  m_connected = true;
  m_timer.stop();
  m_timer.disconnect();
  QObject::connect(this,SIGNAL(serverError(QAbstractSocket::SocketError)),this,SLOT(connectionLost()));
}

void RPCPeer::checkTimeOut(){
    qDebug("RPC Peer: time out while attempting connection!");
    QCoreApplication::exit(-1);
}

void RPCPeer::connectionLost(){
  m_connected = false;  
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
  QxtRPCPeer::connect(m_rpcInfo->serverInfo,m_rpcInfo->serverPort);
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
  attachSignal(m_thread,SIGNAL(reconstructionStarted()),QString("reconstructionStarted()"));
  QObject::connect(m_thread,SIGNAL(finished()),this,SLOT(threadFinished()));
  QObject::connect(m_thread,SIGNAL(reconstructionStarted()),this,SLOT(reconstructionStarted()));
  m_thread->start();
}

void RPCPeer::threadFinished(){
  qDebug("RPCPeer: Reconstruction thread exited.");
  qDebug("RPCPeer: Thread id %p",(void *)QThread::currentThread());
  emit reconstructionStopped();
  QCoreApplication::processEvents();
}

void RPCPeer::reconstructionStarted(){
  qDebug("RPCPeer: Reconstruction started.");
  qDebug("RPCPeer: Thread id %p",(void *)QThread::currentThread());
}

void RPCPeer::stopReconstruction(){
  qDebug("RPCPeer: Stopping reconstruction!");
  m_thread->terminate();
  m_thread->wait();
  reconstructionStopped();
  QCoreApplication::processEvents();
}

void RPCPeer::quit(){
  qDebug("RPCPeer: Quiting!");
  if(m_thread && m_thread->isRunning()){
    m_thread->terminate();
    m_thread->wait();
  }
  QCoreApplication::quit();
}

void RPCPeer::sendMessage(MessageType type,QString s){
  qDebug("RPCPeer: Sending message");
  emit messageSent(type,s);
  QCoreApplication::processEvents();
}

void RPCPeer::sendLogLine(QString s){
  emit logLineSent(s);
  QCoreApplication::processEvents();
}

void RPCPeer::sendImageOutputNotification(QString s){
  /* Check if it's a useful image file */
  if(s.endsWith(".h5") && 
     (s.contains("real_out-") ||
      s.contains("support-"))){
    emit imageOutputNotificationSent(s);
    QCoreApplication::processEvents();
  }
}

bool RPCPeer::isConnected(){
  return m_connected;
}

void RPCPeer::loadImage(QString location){
  qDebug("RPCPeer: Loading %s",location.toAscii().constData());
  Image * a = sp_image_read(location.toAscii().constData(),0);
}
#endif
