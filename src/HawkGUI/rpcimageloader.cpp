#include "rpcimageloader.h"

#include <QtGui>
#include "rpcserver.h"
#include "imagestream.h"

RPCImageLoader::RPCImageLoader(RPCServer * server,QObject * p)
  :QObject(p),m_server(server)
{
}

void RPCImageLoader::receiveImageOutput(quint64 client, QString location,QByteArray data){
  qDebug("RPCImageLoader: Image output received %s",location.toAscii().constData());
  QString workDir = workDirectoryFromClient(client);
  QFileInfo fi(workDir,location);
  ImageStream outstream(&data,QIODevice::ReadOnly);
  Image * a;
  outstream >> a;
  sp_image_write(a,fi.filePath().toAscii().constData(),0);
  if(location.endsWith(".h5")){
    ImageCategory * c = ImageCategory::getFileCategory(location);
    QPair<quint64,ImageCategory *> key = QPair<quint64,ImageCategory *>(client,c);
    if(m_notifications.contains(client)){    
      m_notifications[client] << location;
    }else{
      m_notifications.insert(client,QStringList(location));
    }
    if(!m_notificationsByCategory.contains(key)){
      m_notificationsByCategory.insert(key,QStringList());
      m_notificationsByCategory[key] << location;  
      emit initialImageOutputReceived(c->getName(),fi);
    }else{
      QFileInfo oldFi(workDir,m_notificationsByCategory[key].last());
      m_notificationsByCategory[key] << location;  
      emit imageOutputReceived(c->getName(),fi,oldFi);
    }
  }
}

void RPCImageLoader::loadImage(quint64 client, QString location){
  qDebug("RPCImageLoader: loading %s from %llu",location.toAscii().constData(),client);
  m_server->call(client,QString("loadImage(QString)"),location);
}

void RPCImageLoader::receiveImage(quint64 client, QString location,QByteArray data){
  qDebug("RPCImageLoader: receiving %s from %llu",location.toAscii().constData(),client);
  ImageStream outstream(&data,QIODevice::ReadOnly);
  Image * a;
  outstream >> a;
  emit imageLoaded(client,location,a);
}

QString RPCImageLoader::nextInSequence(quint64 client,QString location){
  ImageCategory * c = ImageCategory::getFileCategory(location);
  QPair<quint64,ImageCategory *> key = QPair<quint64,ImageCategory *>(client,c);
  QString ret;
  if(m_notificationsByCategory.contains(key)){
    int index = m_notificationsByCategory.value(key).indexOf(location);
    if(index >= 0 && index < m_notificationsByCategory.value(key).size()-1){
      ret = m_notificationsByCategory.value(key).at(index+1);
    }
  }
  return ret;
}

void RPCImageLoader::addClient(quint64 client,Options * opts){
  qDebug("RPCImageLoader: adding client");
  m_workDirectoryByClient.insert(client,QString(opts->work_dir));
  m_logFileByClient.insert(client,QString(opts->log_file));
  /* also write configuration file to local directory */
  QFileInfo fi(QDir(opts->work_dir),"uwrapc.confout");
  write_options_file(fi.filePath().toAscii().constData());
}


QString RPCImageLoader::workDirectoryFromClient(quint64 client){
  return m_workDirectoryByClient.value(client);
}

QString RPCImageLoader::logFileFromClient(quint64 client){
  return m_logFileByClient.value(client);
}
