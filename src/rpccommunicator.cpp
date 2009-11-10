#ifdef NETWORK_SUPPORT
#include "rpccommunicator.h"
#include <QCoreApplication>

RPCCommunicator::RPCCommunicator()
:m_connected(false)
{
}

void RPCCommunicator::connectionEstablished(){
  qDebug("RPC Communicator: Connection establish");
  m_connected = true;
}

void RPCCommunicator::checkTimeOut(){
  if(!m_connected){
    qDebug("RPC Communicator: time out while attempting connection!");
    QCoreApplication::exit(-1);
  }
}

void RPCCommunicator::connectionLost(){
  qDebug("RPC Communicator: lost connection to server!");
  m_connected = false;
}



#endif
