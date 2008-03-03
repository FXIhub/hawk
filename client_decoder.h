#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _CLIENT_DECODER_H_ 
#define _CLIENT_DECODER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "decoder.h"

class ClientDecoder: public Decoder
{
  Q_OBJECT
    public:
  ClientDecoder(Communicator * p);
  ClientDecoder * create(Communicator * p);
 private:
  void setVariable(QByteArray command);
  void readData(QByteArray command);
  void readImage(QString id, qulonglong sizeRead, QByteArray data);
  void execRun(QByteArray command);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif


