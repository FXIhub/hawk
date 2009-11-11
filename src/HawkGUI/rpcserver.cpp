#include "rpcserver.h"


RPCServer::RPCServer(int port){
  listen(QHostAddress::Any, port);
  
}
