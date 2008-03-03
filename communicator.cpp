#if defined NETWORK_SUPPORT
#include <QQueue>
#include <QTextStream>
#include <QTimer>
#include <QByteArray>
#include <QMutexLocker>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QSemaphore>
#include <QTime>
#include "communicator.h"
#include "configuration.h"
#include "client_decoder.h"





QMutex Communicator::socketMutex;

Communicator::Communicator(QTcpSocket * s, Decoder * _decoderFactory, Sender * send)
{
  socket = s;
  connect(socket,SIGNAL(readyRead()),this,SLOT(readCommands()));
  decoderFactory = _decoderFactory;
  sender = send;
  sender->setCommunicator(this);
  connect(sender,SIGNAL(writeToSocket(QByteArray)),this,SLOT(writeToSocket(QByteArray)));
}




void Communicator::sendData(VariableMetadata * vm, int id){
  sender->sendData(vm,id);
}

void Communicator::readCommands(){
  /* Make sure we finish handling any command before we 
     allow anyone else to mess with our datastream*/
  socketMutex.lock();
  /* The while is necessary because we might receive more than 1 command at a time*/
  while(socket->bytesAvailable() >= sizeof(int)){
    /* lets read our command */
    int size = (*((int *)socket->read(sizeof(int)).data()));
    while(socket->bytesAvailable() < size){
      socket->waitForReadyRead(-1);
    }
    QByteArray command = socket->read(size);    
    commandQueue.enqueue(command);    
    Decoder * dec = decoderFactory->create(this);
    connect(dec,SIGNAL(dataReceived(int)),this,SLOT(updateData(int)));
    dec->start();
    connect(dec,SIGNAL(finished()),this,SLOT(cleanupDecoders()));
    decoderList.append(dec);
  }    
  socketMutex.unlock();    
}

void Communicator::writeToSocket(QByteArray a){
  qDebug("Sent %d bytes",a.size());
  socket->write(a);
  socket->flush();
}

void Communicator::updateData(int id){
  VariableMetadata * vm = getMetadataById(id);
  if(vm->variable_type == Type_Real){    
    qDebug("Received %s=%f",vm->variable_name,*((real *)vm->variable_address));
  }else if(vm->variable_type == Type_Image){    
    if(*(Image **)vm->variable_address){
      qDebug("Received image %s with dimensions x=%d y=%d z=%d",vm->variable_name,sp_image_x(*(Image **)vm->variable_address),
	     sp_image_y(*(Image **)vm->variable_address),sp_image_z(*(Image **)vm->variable_address));
    }else{
      qDebug("Received null image");
    }
  }else if(vm->variable_type == Type_Int || vm->variable_type == Type_Bool || vm->variable_type == Type_MultipleChoice){    
    qDebug("Received %s=%d",vm->variable_name,*((int *)vm->variable_address));
  }else if(vm->variable_type == Type_String){    
    qDebug("Received %s=%s",vm->variable_name,((char *)vm->variable_address));
  }
  removeIdPair(id);
}


void Communicator::cleanupDecoders(){
  for (int i = 0; i < decoderList.size(); ++i) {
    if (decoderList.at(i)->isFinished()){
      Decoder * old = decoderList.takeAt(i);
      delete old;
    }
  }
}


int Communicator::insertIdPair(VariableMetadata * data){
  int id = qrand();
  /* make sure we get unique keys */
  while(idMap.contains(id)){
    id = qrand();
  }
  idMap.insert(id,data);
  return id;
}

VariableMetadata * Communicator::getMetadataById(int id){
  if(idMap.contains(id)){
    return idMap.value(id);
  }
  return NULL;
}

void Communicator::removeIdPair(int id){
  idMap.remove(id);
}

void Communicator::testSend(){
  sender->sendSetVariable("beta",0.65);  
}

void Communicator::testGet(){
  global_options.beta = 0.1;
  sender->sendExecGetVariable("beta");
  sender->sendExecGetVariable("current_support");
}

void Communicator::testRun(){
  sender->sendExecRun();

}
#endif
