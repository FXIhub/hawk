#include "uwrapcpeerthread.h"
#include "configuration.h"
#include "uwrapc.h"
#include "rpcpeer.h"
#include <QCoreApplication>

UwrapcPeerThread::UwrapcPeerThread(RPCInfo * rpcInfo)
  :m_rpcInfo(rpcInfo)
{
}
 
void UwrapcPeerThread::run(){
  qDebug("UwrapcPeerThread: Thread %p started",(void *)currentThread());
  qDebug("UwrapcPeerThread: Starting run");
  emit reconstructionStarted();
  /* it's necessary to processEvents for the
     signal to reach its destination */
  QCoreApplication::processEvents();
  uwrapc_start(&global_options,m_rpcInfo);
  qDebug("UwrapcPeerThread: finishing run");
}

void UwrapcPeerThread::terminate(){
  setTerminationEnabled(true);
  QThread::terminate();
}
