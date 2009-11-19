#ifndef _PROCESSCONTROL_H_
#define _PROCESSCONTROL_H_

#include <QObject>
#include "configuration.h"
#include <QProcess>
#include <QFileInfo>
#include <QDateTime>

#include <io_utils.h>


class QWidget;
class UwrapcThread;
class RPCServer;

class ProcessControl: public QObject
{
  Q_OBJECT
    public:
  ProcessControl(QWidget * parent = NULL);
  QDateTime startTime();
  enum LaunchMethod{LaunchLocally=1,LaunchRemotely};
  public slots:
  void startProcess();
  void stopProcess();
  void setOptions(Options * opts);
  Options * getOptions();
  void deleteOutputFromDir(QString dir);
  bool isRunning();
  void cleanRemoteClient(quint64 client, int key);
 signals:
  void processFinished();
  void processStarted(QString type, QString workDir,ProcessControl * process);
  void newOutput(QString type, QFileInfo fi);  
  private slots:
  void readStdOut();
  void readStdErr();
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void handleRemoteClient(int key);
  void displayMessage(quint64 client, int type,QString msg);
 private:
  void startLocalProcess();
  void startEmbeddedProcess();
  void startRPCProcess();
  void startRemoteProcessBySSH(int key);
  Options * options;
  QProcess * process;
  QWidget * parent;
  QDateTime p_startTime;
  RPCServer * m_rpcServer;
  QList<int> m_keysToStart;
  QList<int> m_keysRunning;
  LaunchMethod m_processType;
};

#endif
