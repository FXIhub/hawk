#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCPEER_H_ 
#define _RPCPEER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "qxtrpcpeer.h"
#include <QTimer>
#include "io_utils.h"
//#include <QHostInfo>
class RPCPeer;
class UwrapcPeerThread;

struct RPCInfo{
  QString serverInfo;
  int serverPort;
  RPCPeer * peer;
  int key;
};


class RPCPeer: public QxtRPCPeer
{
  Q_OBJECT
    public:
  RPCPeer(RPCInfo * rpcInfo);
  void connect(QString addr , int port);
  void sendLogLine(QString s);
  bool isConnected();
  void sendMessage(MessageType type,QString msg);
  void sendImageOutputNotification(QString s);
 signals:
  void imageOutputNotificationSent(QString s);
  void reconstructionStopped();
  void identificationKeySent(int key);
  void messageSent(int type, QString msg);
  void logLineSent(QString line);
  public slots:
  void connectionEstablished();
  void connectionRecovered();
  void checkTimeOut();
  void connectionLost();
  void attemptReconnection();  
  void quit();
  void loadImage(QString location);
  private slots:
  void receiveOptions(QByteArray optionsFile);
  void startReconstruction();
  void threadFinished();
  void stopReconstruction();
  private slots:
  void reconstructionStarted();
 private:
  QTimer m_timer;
  RPCInfo * m_rpcInfo;  
  UwrapcPeerThread * m_thread;
  bool m_connected;
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif

