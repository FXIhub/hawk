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
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
