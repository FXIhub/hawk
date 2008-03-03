#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _SENDER_H_ 
#define _SENDER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QObject>
#include "configuration.h"


class QTcpSocket;
class Communicator;

class Sender: public QObject
{
  Q_OBJECT    
    public:
  Sender(QTcpSocket * socket);
  void sendSetVariable(QString variable_name,int value);
  void sendSetVariable(QString variable_name,double value);
  void sendSetVariable(QString variable_name,QString value);
  void sendSetVariable(QString variable_name,Image * a);
  void sendExecGetVariable(QString variable_name);
  void sendData(VariableMetadata * vm, int id);
  void sendData(VariableMetadata * vm, Image * a);
  void sendExecRun();
  void setCommunicator(Communicator * c);
 signals:
  void writeToSocket(QByteArray a);
 private:
  QTcpSocket * socket;
  Communicator * communicator;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
