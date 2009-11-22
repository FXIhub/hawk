#ifndef _RPCIMAGELOADER_H_
#define _RPCIMAGELOADER_H_ 1

#include <QObject>
#include <QMap>
#include <QStringList>
#include <spimage.h>
class RPCServer;

class RPCImageLoader: public QObject
{
  Q_OBJECT
    public:
  RPCImageLoader(RPCServer * server,QObject * parent = NULL);
  public slots:
  void loadImage(quint64 client, QString location);
  void receiveImageOutputNotification(quint64 client, QString location);
 signals:
  void imageOutputNotificationReceived(quint64 client, QString location);
  void initialImageOutputNotificationReceived(quint64 client, QString location);
  void imageLoaded(quint64 client, QString location,Image * image);
 private:
  QMap<quint64,QStringList> m_notifications;
  RPCServer * m_server;
};
#endif
