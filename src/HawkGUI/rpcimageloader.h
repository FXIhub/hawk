#ifndef _RPCIMAGELOADER_H_
#define _RPCIMAGELOADER_H_ 1

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <spimage.h>
#include <QFileInfo>
#include "imagecategory.h"
#include "configuration.h"
class RPCServer;

class RPCImageLoader: public QObject
{
  Q_OBJECT
    public:
  RPCImageLoader(RPCServer * server,QObject * parent = NULL);
  QString nextInSequence(quint64 client,QString location);
  void addClient(quint64 client,Options * opts);
  public slots:
  void loadImage(quint64 client, QString location);
  void receiveImageOutput(quint64 client, QString location,QByteArray data);
  void receiveImage(quint64 client,QString location,QByteArray data);
  QString workDirectoryFromClient(quint64 client);
  QString logFileFromClient(quint64 client);
 signals:
  void imageOutputReceived(QString location,QFileInfo f,QFileInfo lastf);
  void initialImageOutputReceived(QString location,QFileInfo f);
  void imageLoaded(quint64 client, QString location,Image * image);
 private:
  QMap<quint64,QStringList> m_notifications;
  QMap<QPair<quint64,ImageCategory *>,QStringList> m_notificationsByCategory;
  RPCServer * m_server;
  QMap<quint64,QString> m_workDirectoryByClient;
  QMap<quint64,QString> m_logFileByClient;
};
#endif
