#ifndef _RPCIMAGELOADER_H_
#define _RPCIMAGELOADER_H_ 1

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <spimage.h>
#include "imagecategory.h"
class RPCServer;

class RPCImageLoader: public QObject
{
  Q_OBJECT
    public:
  RPCImageLoader(RPCServer * server,QObject * parent = NULL);
  QString nextInSequence(quint64 client,QString location);
  public slots:
  void loadImage(quint64 client, QString location);
  void receiveImageOutputNotification(quint64 client, QString location);
  void receiveImage(quint64 client,QString location,QByteArray data);
 signals:
  void imageOutputNotificationReceived(quint64 client, QString location);
  void initialImageOutputNotificationReceived(quint64 client, QString location);
  void imageLoaded(quint64 client, QString location,Image * image);
 private:
  QMap<quint64,QStringList> m_notifications;
  QMap<QPair<quint64,ImageCategory *>,QStringList> m_notificationsByCategory;
  RPCServer * m_server;
};
#endif
