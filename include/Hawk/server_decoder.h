#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _SERVER_DECODER_H_ 
#define _SERVER_DECODER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "../decoder.h"

class ServerDecoder: public Decoder
{
  Q_OBJECT
    public:
  ServerDecoder(Communicator * p);
  ~ServerDecoder();
  ServerDecoder * create(Communicator * p);
 private:
  void setVariable(QByteArray command);
  void readData(QByteArray command);
  void readImage(QString id, qulonglong sizeRead, QByteArray data);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif

