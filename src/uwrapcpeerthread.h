#ifndef _UWRAPC_PEER_THREAD_H_
#define _UWRAPC_PEER_THREAD_H_ 1

#include <QThread>
struct RPCInfo;

 class UwrapcPeerThread : public QThread
 {
   Q_OBJECT
 public:
   UwrapcPeerThread(RPCInfo * rpcInfo);
   void terminate();
 protected:
   void run();
 signals:
  void reconstructionStarted();
 private:
   RPCInfo * m_rpcInfo;
 };

#endif
