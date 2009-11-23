#include "rpcimageloader.h"

#include <QtGui>
#include "rpcserver.h"
#include "imagestream.h"

RPCImageLoader::RPCImageLoader(RPCServer * server,QObject * p)
  :QObject(p),m_server(server)
{
}

void RPCImageLoader::receiveImageOutputNotification(quint64 client, QString location){
  qDebug("RPCImageLoader: Image output notification received");
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
    emit initialImageOutputNotificationReceived(client,location);
  }else{
    m_notificationsByCategory[key] << location;  
    emit imageOutputNotificationReceived(client,location);
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
