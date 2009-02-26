#include "server.h"
#include "mainwindow.h"
#include "../communicator.h"
#include "server_decoder.h"
#include "../sender.h"

#include <QTcpServer>
#include <QMessageBox>
#include <QStatusBar>
#include <QTime>
#include <QTimer>

Options global_options;

Server::Server(MainWindow * _mw,int port)
{
  mw = _mw;
  tcpServer = new QTcpServer(this);
  if(!tcpServer->listen(QHostAddress::Any,port)){
    QMessageBox::critical(mw, tr("Hawk"),
			  tr("Unable to start the server: %1.")
			  .arg(tcpServer->errorString()));
    return;
  }
  mw->statusBar()->showMessage(tr("Server now running on port %1\n").arg(tcpServer->serverPort()),5000);
  connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleConnection()));
}

void Server::handleConnection(){
  QTcpSocket *socket = tcpServer->nextPendingConnection();
  ServerDecoder * factory = new ServerDecoder(NULL);
  Sender * sender = new Sender(socket);
  Communicator * communicator = new Communicator(socket,factory,sender);  
  /* this makes sure we clean things up when the client disconnects */
  connect(socket, SIGNAL(disconnected()),
	  socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()),
	  communicator, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()),
	  factory, SLOT(deleteLater()));
  /* testing code */
  communicator->testSend();
  communicator->testRun();
  communicator->testGet();
  QTimer::singleShot(10000, communicator, SLOT(testGet()));
}


  
