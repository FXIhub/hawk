#ifdef NETWORK_SUPPORT
#ifndef _COMMUNICATOR_H_ 
#define _COMMUNICATOR_H_ 1
#ifdef __cplusplus

#include <QObject>
#include <QTcpSocket>

typedef enum{CommandActionSet = 0, CommandActionGet} CommandAction;
/* 
   Commands are composed by an action which can be GET or SET
   followed by a variable name, followed by possibly a new value (in the case of SET)   
*/
typedef struct {
  CommandAction action;
}Command;


class Communicator: public QObject
{
  Q_OBJECT
    
    public:
  Communicator(QTcpSocket * socket);
  private slots:
    void readCommand();
 private:
  void decodeCommand();
  void executeCommand();
  void replyToCommand();

  
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
