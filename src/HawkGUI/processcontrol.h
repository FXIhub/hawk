#ifndef _PROCESSCONTROL_H_
#define _PROCESSCONTROL_H_ 1

#include <QObject>
#include "configuration.h"
#include <QProcess>
#include <QFileInfo>
#include <QDateTime>

#include <io_utils.h>


class QWidget;
class UwrapcThread;
class RPCServer;
class RPCImageLoader;

class ProcessControl: public QObject
{
  Q_OBJECT
    public:
  ProcessControl(QWidget * parent = NULL);
  QDateTime startTime();
  enum LaunchMethod{LaunchLocally=1,LaunchRemotely};
  enum ProcessType{Embedded=1,Local,LocalRPC,NetworkRPC};
  public slots:
  void startProcess();
  void stopProcess();
  void setOptions(Options * opts);
  Options * getOptions();
  void deleteOutputFromDir(QString dir);
  bool isRunning();
  void cleanRemoteClient(quint64 client, int key);
  RPCImageLoader * rpcImageLoader();
 signals:
  void processFinished();
  void processStarted(ProcessControl::ProcessType type, QString workDir,ProcessControl * process);
  void newOutput(QString type, QFileInfo fi);  
  void logLineReceived(QString);
  void firstRPCImageReceived(QString name, Image * image);
  void rpcImageReceived(QString name, Image * image);
  private slots:
  void readStdOut();
  void readStdErr();
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void handleRemoteClient(int key);
  void displayMessage(quint64 client, int type,QString msg);
  void receiveLogLine(quint64 client, QString line);
  void onProcessStarted();
  void onProcessError();
 private:
  bool isValidClient(quint64 client);
  bool isRunningClient(quint64 client);
  bool isStartingClient(quint64 client);
  void startLocalProcess();
  void startEmbeddedProcess();
  void startRPCProcess();
  void startRemoteProcessBySSH(int key);
  Options * options;
  QProcess * process;
  QWidget * parent;
  QDateTime p_startTime;
  RPCServer * m_rpcServer;
  RPCImageLoader * m_rpcImageLoader;
  QList<int> m_keysToStart;
  QList<int> m_keysRunning;
  LaunchMethod m_processType;
};

#endif
