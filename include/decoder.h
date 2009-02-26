#if defined NETWORK_SUPPORT || defined Q_MOC_RUN
#ifndef _DECODER_H_ 
#define _DECODER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QMutex>
#include <QThread>


class Communicator;

class Decoder: public QThread
{
  Q_OBJECT
    public:
  Decoder(Communicator * p);
  void run();
  virtual Decoder * create(Communicator * p);
 signals:
  void dataReceived(int id);
 protected:
  virtual void setVariable(QByteArray command);
  void readData(QByteArray command);
  void readImage(QString id, qulonglong sizeRead, QByteArray data);
  void execGet(QByteArray command);
  virtual void execRun(QByteArray command);
  Communicator * parent;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
#endif
