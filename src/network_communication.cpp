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

/*
  This is to be been as a member variable of this file
  It's just not inside a class because is had to be accessed
  by functions called from the C side of the code.
 */
static RPCInfo * rpcInfo = 0;

void init_qt(int argc, char ** argv){
  qapp = new QCoreApplication(argc,argv);
}

void attempt_connection(char * server, int server_port,int key){
  rpcInfo = new RPCInfo;
  rpcInfo->serverInfo = QString(server);
  rpcInfo->serverPort = server_port;
  rpcInfo->key = key;
  rpcInfo->peer = new RPCPeer(rpcInfo);
  rpcInfo->peer->connect(server, server_port);
}

void rpc_send_message(MessageType type, const char * s){
  if(rpcInfo){
    rpcInfo->peer->sendMessage(type,s);
  }
}

void rpc_send_log_line(const char * s){
  if(rpcInfo){
    rpcInfo->peer->logLine(s);
  }
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


int is_connected(){
  return rpcInfo->peer->isConnected();
}
#endif
