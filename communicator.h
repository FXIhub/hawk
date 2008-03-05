#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _COMMUNICATOR_H_ 
#define _COMMUNICATOR_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QMap>
#include "decoder.h"
#include "sender.h"
#include "configuration.h"



typedef enum{CommandActionSet = 0, CommandActionGet} CommandAction;
/* 
   Commands are composed by an action which can be GET or SET
   followed by a variable name, followed by possibly a new value (in the case of SET)   
*/
typedef struct {
  CommandAction action;
}Command;




/*! This class is in charge of all network communication
  
 *   Hawk communication protocol
 *
 *  - Communication is done through messages
 *  - 4 types of messages: "Set", "Data", "Exec", "Status"   
 *  - All messages end with newline '\n'
 *  - Everything is case sensitive
 *  - Usually variable name corresponds to the the "variable_name" in configuration.c, but for the case of images there is a special notation.
 *    For example image1,z=0 gives the slice from image1 corresponding to z=0. By this method it's possible to extract slices from entire images.
 *
 *  Set messages
 *
 *  Format: Set <variable> <value>
 *
 *  - <variable> must correspond to the "variable_name" in configuration.c
 *  - <value> depends on the variable type. For strings it's a string, for numeric types it's a string representation of the number.
 *    For bool it can be 1 meaning true or 0 meaning false. For multiple choice it's the valid_nme of the option. For all other types
 *    such as images value is a number that correponds to an identifier that has previously been assigned with the "Data" message.
 *
 *
 *  Exec messages
 *
 *  Format: Exec <command> [arg1] [arg2] ... [argn]
 *
 *  - command can be of the following:
 *    - run: starts executing the reconstruction. Takes no arguments.
 *    - get: requests that the receiver sends something to the sender. Takes 2 arguments.
 *      Argument 1 is the variable name. Argument 2  is the unique identifier to be used in the "Data" reply.
 *
 *
 *  Data messages
 *
 *  Format: Data <type> <identifier> <size>
 *          0xxxxxxxxxxxxxxxxxxxxx...xxxx'\n'
 *
 *  N.B.: This message contains at least two newlines. 0xxxxxxx...xxx
 *  represents the binary data that can also contain newlines or even zeroes (NULLs).
 *
 *
 *  - type represents the datatype which can be (check configuration.h for actual value):
 *    - 0 (Type_Real)
 *    - 1 (Type_Int)
 *  - identifier can be either a numerical id that should be unique (this identifier is then used in subsequent "Set" operations),
 *    or it can be a variable_name, in which case that variable is immediately set to the value passed.
 *  - size is simply the size of the binary messages plus the newline, in bytes.
 *  
 *  
 *  Status messages
 *
 *  Format: Status <status>
 *
 *  - This simple message communicates the status of both the server and the client, and all the issued commands.
 *  - status can be one of the following:
 *    - Busy
 *    - Ready
 *    - Done 
 *    - Error
 *
 *
**/
class Communicator: public QObject
{
  Q_OBJECT
    
    public:
  Communicator(QTcpSocket * s, Decoder * factory, Sender * s);
  QQueue<QByteArray> commandQueue;
  /*! Associates a new ID with the given address and return the new ID
   */  
  int insertIdPair(VariableMetadata * data);
  /*! Returns the address associated with a given ID or NULL if the ID does not exist
   */
  VariableMetadata * getMetadataById(int id);
  /*! Removes the ID pair corresponding to the given ID from the database
   */  
  void removeIdPair(int id);
  void sendData(VariableMetadata * vm, int id);
 public slots:
   void writeToSocket(QByteArray data);
  void testSend();
  void testGet();
  void testRun();
 private:
  QTcpSocket * socket;
  QList<Decoder *> decoderList;
  Decoder * decoderFactory;
  Sender * sender;
  QMap<int,VariableMetadata *> idMap;
  static QMutex socketMutex;

 private slots:
  void readCommands();
  void cleanupDecoders();
  void updateData(int id);

};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
