#ifdef NETWORK_SUPPORT

#include "network_communication.h"
#include "communicator.h"
#include "client_decoder.h"
#include <QCoreApplication>
#include <QTcpSocket>
#include "qxtrpcpeer.h"
#include <QHostInfo>
#include "rpcpeer.h"
#include <QTimer>
static QCoreApplication * qapp;


void init_qt(int argc, char ** argv){
  qapp = new QCoreApplication(argc,argv);
}

RPCInfo * attempt_connection(char * server, int server_port,int key){
  RPCInfo * rpcInfo = new RPCInfo;
  rpcInfo->serverInfo = QString(server);
  rpcInfo->serverPort = server_port;
  rpcInfo->key = key;
  rpcInfo->peer = new RPCPeer(rpcInfo);
  rpcInfo->peer->connect(server, server_port);
  return rpcInfo;
}

void setup_signals_and_slots(RPCInfo * rpcInfo){
  //  rpcInfo->comm = new RPCCommunicator;
}

int start_event_loop(){
  /* Start event loop */
  return qapp->exec();
}

void cleanup_and_free_qt(){
  delete qapp;
}

#endif
