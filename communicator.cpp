#include <QQueue>
#include <QTextStream>
#include <QTimer>
#include <QByteArray>
#include <QMutexLocker>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "communicator.h"
#include "configuration.h"

QMutex Decoder::socketReadMutex;

Decoder::Decoder(Communicator * p){
  parent = p;
}

void Decoder::run(){
  socketReadMutex.lock();
  if(!parent->commandQueue.size()){
    /* no data to read, just return */
    socketReadMutex.unlock();
    return;
  }
  QByteArray command = parent->commandQueue.dequeue();
  if(command.startsWith("Data ")){
    QList<QByteArray> tokens = command.split(' ');
    if(tokens.size() != 3){
      fprintf(stderr,"Data command does not contain 3 parts! Discarding data!\n");
      socketReadMutex.unlock();
      return;
    }      
    QString dataId(tokens.at(1));
    qulonglong dataSize(tokens.at(2).trimmed().toULongLong());
    qulonglong dataRead = 0;
    QByteArray data;
    while(dataRead < dataSize){
      if(parent->commandQueue.size() > 0){
	QByteArray new_data = parent->commandQueue.dequeue();
	data.append(new_data);
	dataRead = data.size();
      }
    };    
    /* chop newline out */
    data.chop(1);
  }else if(command.startsWith("Set ")){
    setVariable(command);
  }
  socketReadMutex.unlock();
}

void Decoder::setVariable(QByteArray command){
  /* remove the newline */
  command.chop(1);
  QList<QByteArray> tokens = command.split(' ');
  QString var(tokens.at(1));
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      if(variable_metadata[i].variable_type == Type_String){	
	strcpy((char *)(variable_metadata[i].variable_address),&(command.data()[command.indexOf(var)+var.size()+1]));
      }else if(variable_metadata[i].variable_type == Type_Real){
	if(tokens.size() != 3){
	  qFatal("Set real called without 3 arguments");
	}
	double r = tokens.at(2).toDouble();
	*((real *)variable_metadata[i].variable_address) = r;
      }
    }
  }
}

Communicator::Communicator(QTcpSocket * s)
{
  socket = s;
  connect(socket,SIGNAL(readyRead()),this,SLOT(readCommands()));
}



void Communicator::readCommands(){
  /* Make sure we finish handling any command before we 
     allow anyone else to mess with out datastream*/
  while(socket->canReadLine()){
    QByteArray command = socket->readLine();    
    commandQueue.enqueue(command);    
    Decoder * dec = new Decoder(this);
    dec->start();
    connect(dec,SIGNAL(finished()),this,SLOT(cleanupDecoders()));
    decoderList.append(dec);
  }    
}



void Communicator::cleanupDecoders(){
  for (int i = 0; i < decoderList.size(); ++i) {
    if (decoderList.at(i)->isFinished()){
      Decoder * old = decoderList.takeAt(i);
      delete old;
    }
  }
}
