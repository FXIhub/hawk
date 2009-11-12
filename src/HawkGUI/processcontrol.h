#ifndef _PROCESSCONTROL_H_
#define _PROCESSCONTROL_H_

#include <QObject>
#include "configuration.h"
#include <QProcess>
#include <QFileInfo>
#include <QDateTime>

class QWidget;
class UwrapcThread;
class RPCServer;

class ProcessControl: public QObject
{
  Q_OBJECT
    public:
  ProcessControl(QWidget * parent = NULL);
  QDateTime startTime();
  public slots:
  void startProcess();
  void stopProcess();
  void setOptions(Options * opts);
  Options * getOptions();
  void deleteOutputFromDir(QString dir);
  bool isRunning();
 signals:
  void processFinished();
  void processStarted(QString type, QString workDir,ProcessControl * process);
  void newOutput(QString type, QFileInfo fi);
  private slots:
  void readStdOut();
  void readStdErr();
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
 private:
  void startLocalProcess();
  void startEmbeddedProcess();
  void startRPCProcess();
  Options * options;
  QProcess * process;
  QWidget * parent;
  QDateTime p_startTime;
  RPCServer * m_rpcServer;
};

#endif
