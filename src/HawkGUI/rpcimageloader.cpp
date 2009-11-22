#include "rpcimageloader.h"

#include <QtGui>
#include "rpcserver.h"

RPCImageLoader::RPCImageLoader(RPCServer * server,QObject * p)
  :QObject(p),m_server(server)
{
}

void RPCImageLoader::receiveImageOutputNotification(quint64 client, QString location){
  qDebug("RPCImageLoader: Image output notification received");
  if(m_notifications.contains(client)){    
    m_notifications[client] << location;
    emit imageOutputNotificationReceived(client,location);
  }else{
    m_notifications.insert(client,QStringList(location));
    emit initialImageOutputNotificationReceived(client,location);
  }
}

void RPCImageLoader::loadImage(quint64 client, QString location){
  qDebug("RPCImageLoader: loading %s from %llu",location.toAscii().constData(),client);
  m_server->call(client,QString("loadImage(QString)"),location);
}
