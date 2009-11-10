#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _RPCCOMMUNICATOR_H_ 
#define _RPCCOMMUNICATOR_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QObject>

class RPCCommunicator: public QObject
{
  Q_OBJECT
    public:
  RPCCommunicator();
  public slots:
  void connectionEstablished();
  void checkTimeOut();
  void connectionLost();
 private:
  bool m_connected;
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif

