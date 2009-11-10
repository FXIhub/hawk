#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCPEER_H_ 
#define _RPCPEER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "qxtrpcpeer.h"
#include <QTimer>

class RPCPeer: public QxtRPCPeer
{
  Q_OBJECT
    public:
  RPCPeer();
  void connect(QHostAddress addr , int port);
  public slots:
  void connectionEstablished();
  void checkTimeOut();
  void connectionLost();
  void attemptReconnection();  
 private:
  QTimer m_timer;
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif

