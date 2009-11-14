#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCPEER_H_ 
#define _RPCPEER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "qxtrpcpeer.h"
#include "rpcdefaultport.h"


class RPCServer: public QxtRPCPeer
{
  Q_OBJECT
    public:
  RPCServer(int port=rpcDefaultPort);
  quint64 clientFromKey(int key);
  int keyFromClient(quint64 client);
  void stopByKey(int key);
 signals:
  void keyReceived(int key);
  void clientFinished(quint64 client, int key);
  public slots:
  void sendOptions(quint64 client);
  void startReconstruction(quint64 client);
  void onClientConnected(quint64 client);
  void reconstructionStarted(quint64 client);
  void reconstructionStopped(quint64 client);
  void receiveIdentificationKey(quint64 client, int key);
 private:
  QMap<quint64,int> m_clientKeyMap;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
