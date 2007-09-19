/*#ifdef NETWORK_SUPPORT*/
#ifndef _COMMUNICATOR_H_ 
#define _COMMUNICATOR_H_ 1
/*#ifdef __cplusplus*/

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QThread>
#include <QQueue>


/*
  Description of the communication protocol 

  The protocol is composed of small human readable messages.
  All messages terminal with a newline character (\n)
  The current messages are of 4 types:
  
  Greetings messages - "HELLO FROM <program name>"
  Exit messages - "GOODBYE FROM <program name>"  
  Variable description message - "VARIABLE <variable_name> <metadata> <metadata_value>"
  Where variable_name is one of those described in configuration.c
  And metadata is one of the following:
  Type,Id,Parent,Properties,list_valid_values,list_valid_names,variable_address,memory_address;  
  
  Variable set message - "SET <variable> <value>"
  Variable get message - "GET <variable>"
  
  All values are passed as strings that have to be 
  reinterpreted on the other side unless explicitly noted

  If a value is an array it should be separated by commas and
  commas should never be a part of a value

*/


typedef enum{CommandActionSet = 0, CommandActionGet} CommandAction;
/* 
   Commands are composed by an action which can be GET or SET
   followed by a variable name, followed by possibly a new value (in the case of SET)   
*/
typedef struct {
  CommandAction action;
}Command;


class Communicator;

class Decoder: public QThread
{
  Q_OBJECT
    public:
  Decoder(Communicator * p);
  void run();
 private:
  void setVariable(QByteArray command);
  Communicator * parent;
  static QMutex socketReadMutex;
};

class Communicator: public QObject
{
  Q_OBJECT
    
    public:
  Communicator(QTcpSocket * s);
  QQueue<QByteArray> commandQueue;
  private slots:
  void readCommands();
  void cleanupDecoders();
 private:
  QTcpSocket * socket;
  QList<Decoder *> decoderList;

};
/*#else
#error "Someone is including " __FILE__ " from a C file!"
#endif*/
#endif
/*#endif*/
