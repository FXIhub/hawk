#ifdef NETWORK_SUPPORT

#include "network_communication.h"
#include "communicator.h"
#include "client_decoder.h"
#include <QCoreApplication>
#include <QTcpSocket>
#include "qxtrpcpeer.h"
#include <QHostInfo>
#include "rpccommunicator.h"
#include "rpcpeer.h"
#include <QTimer>
static QCoreApplication * qapp;

struct RPCInfo{
  QHostInfo serverInfo;
  int serverPort;
  RPCPeer * peer;
  RPCCommunicator * comm;
};

void init_qt(int argc, char ** argv){
  qapp = new QCoreApplication(argc,argv);
}

RPCInfo * attempt_connection(char * server, int server_port){
  RPCInfo * rpcInfo = new RPCInfo;
  rpcInfo->serverInfo = QHostInfo::fromName(server);
  rpcInfo->serverPort = server_port;
  if(rpcInfo->serverInfo.error() != QHostInfo::NoError){
    QString errorString = rpcInfo->serverInfo.errorString();
    fprintf(stderr,"Connection failed: %s\n",errorString.toAscii().data());
    delete rpcInfo;
    return NULL;
  }
  rpcInfo->peer = new RPCPeer;
  rpcInfo->peer->connect(rpcInfo->serverInfo.addresses().first(), server_port);
  return rpcInfo;
}

void setup_signals_and_slots(RPCInfo * rpcInfo){
  rpcInfo->comm = new RPCCommunicator;
}

int start_event_loop(){
  /* Start event loop */
  return qapp->exec();
}

void cleanup_and_free_qt(){
  delete qapp;
}

#endif
