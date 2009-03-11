#include "processcontrol.h"
#include <QtGui>
#include <QProcess>
#include <QMessageBox>
#include <QWidget>

#include "outputwatcher.h"
#include "uwrapcthread.h"

ProcessControl::ProcessControl(QWidget * p)
  :QObject(p)
{
  process = NULL;
  parent = p;
}

void ProcessControl::startProcess(){
  qDebug("start");
  //  startLocalProcess();
  startEmbeddedProcess();
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
  process->start(command);
  bool ok;
  ok = process->waitForStarted(1000);
  if(!ok){
    QMessageBox::warning(parent, tr("uwrapc message"),tr("Could not start %1. Please report the situation to the developers").arg(command));
  }else{
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
  process->terminate();
  if(!process->waitForFinished(2000)){
    qDebug("Killing process");
    process->kill();
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
