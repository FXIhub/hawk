#ifndef _UWRAPCTHREAD_H_
#define _UWRAPCTHREAD_H_ 1

#include <QThread>
#include "configuration.h"

class UwrapcThread: public QThread
{
  Q_OBJECT
 public:
  UwrapcThread(Options * opts, QObject * parent);
 protected:
  void run();
  Options * options;
};

#endif
