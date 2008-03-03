#ifndef _SERVER_H_
#define _SERVER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include "../configuration.h"

#include <QObject>

class QTcpServer;
class Sender;
class MainWindow;

class Server: public QObject
{
  Q_OBJECT
    public:
  Server(MainWindow * mw, int  port);
 private:
  QTcpServer * tcpServer;
  MainWindow * mw;
 private slots:
  void handleConnection();
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
