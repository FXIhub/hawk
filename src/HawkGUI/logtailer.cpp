#include "logtailer.h"
#include <QRegExp>
#include <QStringList>
#include <QFileInfo>
#include <QTimer>

LogTailer::LogTailer(QObject * parent)
  :QFileSystemWatcher(parent)
{
  pos = 0;
  connect(this,SIGNAL(fileChanged(QString)),this,SLOT(readLine(QString)));
}


void LogTailer::tailLogFile(QString path){
  qDebug("Watching %s",path.toAscii().data());
  if(QFileInfo(path).exists()){
    addPath(path); 
  }else{
    pathToAdd = path;
    QTimer::singleShot(1000, this, SLOT(tryAddingPath()));
  }
}


void LogTailer::tryAddingPath(){
  if(QFileInfo(pathToAdd).exists()){
    addPath(pathToAdd); 
    pathToAdd = QString();
  }else{
    QTimer::singleShot(10000, this, SLOT(tryAddingPath()));
  }
}
void LogTailer::readLine(QString path){
  //  qDebug("Reading line");
  if(!log.isOpen()){
    log.setFileName(path);
    if(!log.open(QIODevice::ReadOnly)){
      qDebug("Cannot open %s",path.toAscii().data());
      return;
    }    
    reader.setDevice(&log);
  }
  /*  if(!reader.seek(pos)){
    qDebug("Cannot seek log file");
    }*/
  QString line;
  do{
    line = reader.readLine();
    /*    pos = reader.pos();*/
    if(line.isNull()){
      return;
    }
    parseLine(line);
  }while(!line.isNull());
}

void LogTailer::parseLine(QString line){
  QRegExp comment = QRegExp("^\\s*#");
  QRegExp metadata = QRegExp("^\\s*@");
  QRegExp data = QRegExp("^\\s*\\d");
  QRegExp header = QRegExp("^\\s*@\\s*s(\\d+)\\s+legend\\s+\"(.*)\"");
  // check if it's data or metadata
  if(line.contains(comment)){
    // comment
  } else if(line.contains(metadata)){
    // metadata 
    if(line.contains(header)){
      int col = header.cap(1).toInt();
      QString title = header.cap(2);
      emit headerRead(title,col);
    }    
  }else if(line.contains(data)){
    // data
    QStringList fields = line.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<double> data;
    int size = fields.size();
    for(int i = 0;i<size;i++){
      bool ok = false;
      double num = fields.at(i).toDouble(&ok);
      if(ok){
	data.append(num);
      }
    }
    if(data.size()){
      emit dataLineRead(data);
    }
  }
}
