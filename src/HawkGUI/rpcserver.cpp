#include "rpcserver.h"
#include <QtGui>
#include "configuration.h"
#include "imagestream.h"


RPCServer::RPCServer(int port){

  QSettings settings;
  bool ok = false;
  while(!ok){
    ok = listen(QHostAddress::Any, port);
    if(!ok && port < rpcMaxPort){
      /* Try going for higher ports until we can listen to one */
      qDebug("RPCServer: Failed to bind to port %d. Trying higher ports...",port);
      port++;
    }else{
      qDebug("RPCServer: Listening on port %d",port);
      settings.setValue("RPCServer/serverPort",port);
    }
  }  
  if(!ok){
    qDebug("RPCServer: All ports exhausted. Binding failed.");
    return;
  }
  attachSlot(QString("reconstructionStarted()"),this,SLOT(reconstructionStarted(quint64)));
  attachSlot(QString("reconstructionStopped()"),this,SLOT(reconstructionStopped(quint64)));
  attachSlot(QString("identificationKeySent(int)"),this,SLOT(receiveIdentificationKey(quint64,int)));
  QObject::connect(this,SIGNAL(clientConnected(quint64)),this,SLOT(onClientConnected(quint64)));
  QObject::connect(this,SIGNAL(clientDisconnected(quint64)),this,SLOT(onClientDisconnected(quint64)));
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
    sendInputImages(client);
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

void RPCServer::reconstructionStopped(quint64 client){
  qDebug("Reconstruction stopped at peer %llu",client);
  call(client,QString("quit()"));
}

void RPCServer::onClientConnected(quint64 client){
  qDebug("Peer %llu connected",client);
}

void RPCServer::onClientDisconnected(quint64 client){
  qDebug("Peer %llu disconnected",client);
  if(m_clientKeyMap.contains(client)){
    emit clientFinished(client,keyFromClient(client));
    m_clientKeyMap.remove(client);
  }
}

void RPCServer::receiveIdentificationKey(quint64 client,int key){
  qDebug("Peer %llu sent key %d",client,key); 
  m_clientKeyMap.insert(client,key);
  emit keyReceived(key);
}

quint64 RPCServer::clientFromKey(int key){
  return m_clientKeyMap.key(key);
}

int RPCServer::keyFromClient(quint64 client){
  return m_clientKeyMap.value(client);
}

void RPCServer::stopByKey(int key){
  quint64 client = clientFromKey(key);
  call(client,QString("stopReconstruction()"));
}

void RPCServer::sendInputImages(quint64 client){
  if(global_options.diffraction_filename[0]){
    Image * a = sp_image_read(global_options.diffraction_filename,0);
    if(!a){
      qCritical("RPCServer: Could not open amplitudes file!");
      return;
    }
    QByteArray data;
    ImageStream instream(&data,QIODevice::WriteOnly);
    instream << a;
    call(client,QString("inputImageSent(QByteArray,QString)"),data,"amplitudes");
    sp_image_free(a);
  }
  if(global_options.real_image_filename[0]){
    Image * a = sp_image_read(global_options.real_image_filename,0);
    if(!a){
      qCritical("RPCServer: Could not open real image file!");
      return;
    }
    QByteArray data;
    ImageStream instream(&data,QIODevice::WriteOnly);
    instream << a;
    call(client,QString("inputImageSent(QByteArray,QString)"),data,"real_image");
    sp_image_free(a);
  }
}
