#ifdef NETWORK_SUPPORT

#include "network_communication.h"
#include "communicator.h"
#include <QCoreApplication>
#include <QTcpSocket>
static QCoreApplication * qapp;

void init_qt(int argc, char ** argv){
  qapp = new QCoreApplication(argc,argv);
}

void * attempt_connection(char * server, int server_port){
  QTcpSocket * socket = new QTcpSocket;
  socket->connectToHost(server,server_port);
  if(socket->waitForConnected()){
    printf("Valid socket!\n");
    return socket;
  }
  delete socket;
  return NULL;
}

void wait_for_server_instructions(void * _socket){
  QTcpSocket * socket = reinterpret_cast<QTcpSocket *>(_socket);
  Communicator communicator(socket);  
  /* Start event loop */
  qapp->exec();
  
}

void cleanup_qt(){
  delete qapp;
}

#endif
