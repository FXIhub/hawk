#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCPEER_H_ 
#define _RPCPEER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "qxtrpcpeer.h"
#include <QTimer>
#include <QHostInfo>

class RPCPeer;
class UwrapcPeerThread;

struct RPCInfo{
  QHostInfo serverInfo;
  int serverPort;
  RPCPeer * peer;
  int key;
};


class RPCPeer: public QxtRPCPeer
{
  Q_OBJECT
    public:
  RPCPeer(RPCInfo * rpcInfo);
  void connect(QHostAddress addr , int port);
 signals:
  void identificationKey(int key);
  void reconstructionStopped();
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
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif

