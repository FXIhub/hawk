#include "processcontrol.h"
#include <QtGui>
#include <QProcess>
#include <QMessageBox>
#include <QWidget>

#include "outputwatcher.h"
#include "uwrapcthread.h"
#include "rpcserver.h"
#include "remotelaunchdialog.h"

ProcessControl::ProcessControl(QWidget * p)
  :QObject(p)
{
  process = NULL;
  parent = p;
  m_rpcServer = new RPCServer();
  connect(m_rpcServer,SIGNAL(keyReceived(int)),this,SLOT(handleRemoteClient(int)));
  connect(m_rpcServer,SIGNAL(clientFinished(quint64,int)),this,SLOT(cleanRemoteClient(quint64,int)));
  connect(m_rpcServer,SIGNAL(warningMessage(int,QString)),this,SLOT(handleWarningMessage(int,QString)));
  connect(m_rpcServer,SIGNAL(criticalMessage(int,QString)),this,SLOT(handleCriticalMessage(int,QString)));
}

void ProcessControl::startProcess(){
  QSettings settings;
  qDebug("start");
  if(settings.value("ProcessControl/launchMethod") == ProcessControl::LaunchLocally){
  //  startLocalProcess();
    startEmbeddedProcess();
  }else if(settings.value("ProcessControl/launchMethod") == ProcessControl::LaunchRemotely){
    startRPCProcess();
  }
}

void ProcessControl::startRPCProcess(){
  qDebug("ProcessControl: Starting remote process");
  int key;
  /* Make sure the generated key is unique */
  do{
    key = qrand();
  }while(m_keysToStart.contains(key));
  m_keysToStart.append(key);
  startRemoteProcessBySSH(key);
}

void ProcessControl::startRemoteProcessBySSH(int key){
  QSettings settings;
  QString selectedProfile = settings.value("RemoteLaunchDialog/selectedProfile").toString();
  QString remoteHost = settings.value("RemoteLaunchDialog/"+selectedProfile+"/remoteHost").toString();
  QString sshPath = settings.value("RemoteLaunchDialog/"+selectedProfile+"/sshPath").toString();
  QString uwrapcPath = settings.value("RemoteLaunchDialog/"+selectedProfile+"/uwrapcPath").toString();
  int remotePort = settings.value("RemoteLaunchDialog/"+selectedProfile+"/remotePort").toInt();
  QString localHost = settings.value("RemoteLaunchDialog/"+selectedProfile+"/localHost").toString();
  int localPort = RemoteLaunchDialog::getLocalPortNumber();
  QString command = QString("%1 -p %2 -t %3 %4 %5 %6 %7").arg(sshPath).arg(remotePort).
    arg(remoteHost).arg(uwrapcPath).arg(localHost).arg(localPort).arg(key);
  qDebug("ProcessControl: running '%s'",command.toAscii().constData());
  QProcess::startDetached(command);  
}

void ProcessControl::startLocalProcess(){
  /* I'm always gonna start the process in the same location as the output directory */
  /* I'm also gonna assume that hawk is in the path */
  if(process){
    delete process;
  }
  process = new QProcess(this);
  QString fullPath = QFileInfo(options->work_dir).absoluteFilePath();
  process->setWorkingDirectory(fullPath);
  /* write configuration file */
  QString filename = QString(fullPath)+QString("/uwrapc.conf");
  write_options_file(filename.toAscii().data());
  connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readStdOut()));
  connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readStdErr()));
  connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onProcessFinished(int,QProcess::ExitStatus)));
  p_startTime = QDateTime::currentDateTime();
  process->start("uwrapc");
  emit processStarted("local",fullPath,this);  
}

void ProcessControl::startEmbeddedProcess(){
  /* I'm always gonna start the process in the same location as the output directory */
  /* I'm also gonna assume that hawk is in the path */
  if(process){
    delete process;
  }
  process = new QProcess(this);
  QString fullPath = QFileInfo(options->work_dir).absoluteFilePath();
  process->setWorkingDirectory(fullPath);
  /* write configuration file */
  QString filename = QString(fullPath)+QString("/uwrapc.conf");
  write_options_file(filename.toAscii().data());
  connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readStdOut()));
  connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readStdErr()));
  connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onProcessFinished(int,QProcess::ExitStatus)));
  QString programName = QCoreApplication::arguments().at(0);
  if(QDir::fromNativeSeparators(programName).contains("/")){
    /* program given with full path */
    programName = QFileInfo(programName).absoluteFilePath();
  }
  QString command = programName+QString(" uwrapc");
  qDebug("Starting %s",command.toAscii().data());
  p_startTime = QDateTime::currentDateTime();
  QStringList arguments("uwrapc");
  process->start(programName,arguments);
  bool ok;
  ok = process->waitForStarted(1000);
  if(!ok){
    QMessageBox::warning(parent, tr("uwrapc message"),tr("Could not start %1. Please report the situation to the developers").arg(command));
  }else{
    m_processType = LaunchLocally;  
    emit processStarted("embedded",fullPath,this);  
  }
}


/*void ProcessControl::startEmbeddedProcess(){
  if(uwrapcThread){
    delete uwrapcThread;
  }
  uwrapcThread = new UwrapcThread(options,this);
*/
  /*  connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(readStdOut()));
  connect(process,SIGNAL(readyReadStandardError()),this,SLOT(readStdErr()));
  connect(process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onProcessFinished(int,QProcess::ExitStatus)));*/

//  uwrapcThread->start();
//  emit processStarted("embedded",QString(),this);  
//}

void ProcessControl::stopProcess(){  
  if(m_processType == LaunchLocally){
    if(process){
      process->terminate();
      if(!process->waitForFinished(2000)){
	qDebug("Killing process");
	process->kill();
      }
    }
  }else if(m_processType == LaunchRemotely){
    if(m_keysRunning.size()){
      m_rpcServer->stopByKey(m_keysRunning.first());
    }
  }
}

void ProcessControl::setOptions(Options * opts){
  //  qDebug("New options");
  options = opts;
}


void ProcessControl::readStdOut(){
  qDebug("stdout");
  QByteArray out = process->readAllStandardOutput();
  QMessageBox::warning(parent, tr("uwrapc message"),QString(out));
}
void ProcessControl::readStdErr(){  
  qDebug("stderr");
  QByteArray err = process->readAllStandardError();
  QMessageBox::critical(parent, tr("uwrapc message"),QString(err));
  
}

void ProcessControl::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus){
  if(exitCode){
    if(exitStatus == QProcess::NormalExit){
      QMessageBox::information(parent, tr("uwrapc message"),tr("uwrapc exited normally with exit code %1").arg(exitCode));
    }
    if(exitStatus == QProcess::QProcess::CrashExit){
      QMessageBox::information(parent, tr("uwrapc message"),tr("uwrapc seems to have crashed with exit code %1. Please report the situation to the developers").arg(exitCode));
    }
  }
  emit processFinished();
}

Options * ProcessControl::getOptions(){
  return options;
}


void ProcessControl::deleteOutputFromDir(QString dir_path){
  QDir dir(dir_path);
  QStringList filters;
  filters << "amplitudes-*.h5" << "pattern-*.h5" << "real_out-*.h5" << "real_out_phase-*.h5" << "support-*.h5";
  filters << "amplitudes-*.png" << "pattern-*.png" << "real_out-*.png" << "real_out_phase-*.png" << "support-*.png";
  filters << "amplitudes-*.vtk" << "pattern-*.vtk" << "real_out-*.vtk" << "real_out_phase-*.vtk" << "support-*.vtk";
  dir.setNameFilters(filters);
  QStringList filesToDelete = dir.entryList();
  int size = filesToDelete.size();
  for(int i = 0;i<size;i++){
    dir.remove(filesToDelete.at(i));
  }
}

QDateTime ProcessControl::startTime(){
  return p_startTime;
}


bool ProcessControl::isRunning(){
  if(process){
    QProcess::ProcessState state = process->state();
    if(state == QProcess::NotRunning){
      return false;
    }else{
      return true;
    }       
  }
  return false;
}

void ProcessControl::handleRemoteClient(int key){
  if(m_keysToStart.contains(key)){
    m_keysToStart.removeAll(key);
    m_keysRunning.append(key);
    qDebug("ProcessControl: Starting reconstruction on client with key %d",key);
    quint64 client = m_rpcServer->clientFromKey(key);
    m_rpcServer->sendOptions(client);
    m_rpcServer->startReconstruction(client);    
    m_processType = LaunchRemotely;
  }else{
    qDebug("ProcessControl: Received unknown key - %d",key);
  }
}

void ProcessControl::cleanRemoteClient(quint64 client, int key){
  qDebug("ProcessControl: Cleaning finished client %llu",client);
  QMessageBox::information(0, tr("HawkGUI"),
			   tr("Remote process finished."),
			   QMessageBox::Ok,QMessageBox::Ok);
  
  m_keysRunning.removeAll(key);
  emit processFinished();
}

void ProcessControl::handleWarningMessage(int key, QString msg){
  if(m_keysToStart.contains(key) || m_keysRunning.contains(key)){
    QMessageBox::warning(0, tr("HawkGUI"),
			 msg,
			 QMessageBox::Ok,QMessageBox::Ok);
  }
}

void ProcessControl::handleCriticalMessage(int key, QString msg){
  if(m_keysToStart.contains(key) || m_keysRunning.contains(key)){
    QMessageBox::critical(0, tr("HawkGUI"),
			  msg,
			  QMessageBox::Ok,QMessageBox::Ok);
  }
}
