#include "uwrapcthread.h"
#include "uwrapc.h"
#include <QThread>

UwrapcThread::UwrapcThread(Options * opts,QObject * parent)
  :QThread(parent)
{
  options = opts;
}

void UwrapcThread::run(){
  init_reconstruction(options);
}
