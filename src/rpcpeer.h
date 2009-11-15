#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCPEER_H_ 
#define _RPCPEER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "qxtrpcpeer.h"
#include <QTimer>
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
  void warningMessage(QString s);
  void criticalMessage(QString s);
  void infoMessage(QString s);
  void logLine(QString s);
  bool isConnected();
 signals:
  void identificationKey(int key);
  void reconstructionStopped();
  void sendWarningMessage(QString msg);
  void sendCriticalMessage(QString msg);
  void sendInfoMessage(QString msg);
  void sendLogLine(QString line);
  public slots:
  void connectionEstablished();
  void connectionRecovered();
  void checkTimeOut();
  void connectionLost();
  void attemptReconnection();  
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

