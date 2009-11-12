#include "rpcserver.h"
#include <QtGui>
#include "configuration.h"


RPCServer::RPCServer(int port){
  bool ok = false;
  while(!ok){
    ok = listen(QHostAddress::Any, port);
    if(!ok && port < rpcMaxPort){
      /* Try going for higher ports until we can listen to one */
      qDebug("RPCServer: Failed to bind to port %d. Trying higher ports...",port);
      port++;
    }else{
      qDebug("RPCServer: Listening on port %d",port);
    }
  }  
  if(!ok){
    qDebug("RPCServer: All ports exhausted. Binding failed.");
    return;
  }
  attachSlot(QString("reconstructionStarted()"),this,SLOT(reconstructionStarted(quint64)));
  QObject::connect(this,SIGNAL(clientConnected(quint64)),this,SLOT(onClientConnected(quint64)));
  //  attachSignal(this,SIGNAL(sendOptions(quint64,QByteArray)),QString("sendOptions(quint64,QByteArray)"));
  //  attachSignal(this,SIGNAL(startReconstruction(quint64)),QString("startReconstruction(quint64)"));
}


void RPCServer::sendOptions(quint64 client){
  QTemporaryFile file;
  if (file.open()) {
    file.close();
    write_options_file(file.fileName().toAscii().constData());
    file.open();
    QByteArray optionsString = file.readAll();
    qDebug("Read %d bytes in the configuration file",optionsString.size());
    call(client,QString("sendOptions(QByteArray)"),optionsString);
  }else{
    qDebug("RPCServer: Failed to create temporary file!");
    return;
  }
}

void RPCServer::startReconstruction(quint64 client){
  call(client,QString("startReconstruction()"));
}

void RPCServer::reconstructionStarted(quint64 client){
  qDebug("Reconstruction started at peer %llu",client);
}

void RPCServer::onClientConnected(quint64 client){
  qDebug("Peer %llu connected",client);
}
