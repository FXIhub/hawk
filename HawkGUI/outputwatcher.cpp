#include "outputwatcher.h"
#include "imagecategory.h"
#include <QtGui>

OutputWatcher::OutputWatcher(QString outputDir, QObject * parent,QList<ImageCategory *> * ic,int inc,QDateTime startTime)
//  :QFileSystemWatcher(parent)
  :QThread(parent)
{
  //   addPath (outputDir);
  /* For some reason QFileSystemWatcher doesn't seem to work */
  initTime = startTime;
  increment = inc;
  imageCategories = ic;
  pooler = new QTimer;
  connect(pooler,SIGNAL(timeout()),this,SLOT(checkForNewFiles()));
  //  connect(this,SIGNAL(directoryChanged(const QString &)),this,SLOT(checkForNewFiles()));
  setupQDir(outputDir);
}

void OutputWatcher::setupQDir(QString path){
  dir = QDir(path);
  QStringList filters;
  filters << "*.h5";
  //  filters << "*.png";
  dir.setNameFilters(filters);  
  dir.setFilter(QDir::Files|QDir::Readable);
  //  dir.setSorting(QDir::Time);
  //  checkForNewFiles();
}

/*
void OutputWatcher::checkForNewFiles(){
  QStringList currentFiles = dir.entryList();
  int size = currentFiles.size();
  for(int i = 0;i<size;i++){
    if(outputFiles.contains(QFileInfo(dir,currentFiles[i]))){
      continue;
    }else{
      processFile(currentFiles[i]);
    }
  }  
}
*/
void OutputWatcher::checkForNewFiles(){
  QFileInfo fi;
  QString file;
  QFileInfo previousFile;
  while(1){
    if(newestFiles.contains("Object")){
      fi = newestFiles.value("Object");
      if(previousFile == fi){
	return;
      }
      /* We have a previous object, lets use the increment to check for the next */
      file = incrementFilename(fi.absoluteFilePath(),increment);    
    }else{
      QString baseIter = "real_out-0000000.h5";
      fi = QFileInfo(dir,baseIter);
      file = incrementFilename(fi.absoluteFilePath(),increment-1);    
    }
    qDebug(("Processing " + file).toAscii());
    previousFile = fi;
    ImageCategory * ic = ImageCategory::getFileCategory(file);
    if(processFile(file)){
      return;
    }
    for(int i = 0;i<imageCategories->size();i++){
      if(ic == imageCategories->at(i)){
	continue;
      }
      QString trans = ImageCategory::translateFilename(file,ic,imageCategories->at(i));
      if(QFileInfo(trans).exists()){
	processFile(trans);
      }
    }
  }
}

int OutputWatcher::processFile(const QString file){
  QString key;
  
  ImageCategory * ic = ImageCategory::getFileCategory(file);
  if(!ic){
    //    qDebug(("No image category for "+ file).toAscii());
    return -1;
  }
  key = ic->getName();

  if(!isFileValid(file)){
    //    qDebug((file+" not valid").toAscii());
    return -2;
  }
  QFileInfo fileInfo(file);
  if(!outputFiles.contains(fileInfo)){
    outputFiles.append(fileInfo);
  }
  if(newestFiles.contains(key)){    
    //    qDebug("File already exists in keys");
    if(fileInfo.created() >= newestFiles.value(key).created()){
      emit newOutput(key,fileInfo,newestFiles.value(key));
      newestFiles.insert(key,fileInfo);
      return 0;
    }
  }else{
    //    qDebug("File does not exists in keys");
    newestFiles.insert(key,fileInfo);
    emit newOutput(key,fileInfo,QFileInfo());
    //    qDebug("Initial Output emited");
    emit initialOutput(key,fileInfo);
    return 0;
  }
  return -3;
}

void OutputWatcher::stop(){
  if(pooler){
    pooler->stop();
  }
}

QList<QFileInfo> OutputWatcher::getOutputFiles(){
  return outputFiles;
}

bool OutputWatcher::fileExists(QString file){
  for(int i = 0;i<outputFiles.size();i++){
    if(outputFiles.at(i).fileName() == file ||outputFiles.at(i).absoluteFilePath() == file ){
      return true;
    }
  }
  return false;
}

QFileInfo OutputWatcher::getFileInfo(QString file){
  for(int i = 0;i<outputFiles.size();i++){
    if(outputFiles.at(i).fileName() == file ||outputFiles.at(i).absoluteFilePath() == file){
      return outputFiles.at(i);
    }
  }
  return QFileInfo();
}

QFileInfo OutputWatcher::getNextFile(QString file){
  QFileInfo fi = getFileInfo(file);
  QFileInfo ret = QFileInfo();
  if(fi == QFileInfo()){
    return ret;
  }
  ImageCategory * ic = ImageCategory::getFileCategory(file);
  if(!ic){
    return ret;
  }
  for(int i = 0;i<outputFiles.size();i++){
    // Files must have the same ending
    if(outputFiles.at(i).suffix() == fi.suffix()){
      // Files must be of the same category
      if(ic->includes(outputFiles.at(i).fileName())){
	// File must be newer than current file
	if(outputFiles.at(i).created() > fi.created()){
	  if(ret.fileName().isEmpty()){
	    ret = outputFiles.at(i);
	  }else{
	    if(outputFiles.at(i).created() < ret.created()){
	      ret = outputFiles.at(i);
	    }
	  }
	}
      }
    }  
  }
  return ret;
}

QFileInfo OutputWatcher::getPreviousFile(QString file){
  QFileInfo fi = getFileInfo(file);
  QFileInfo ret = QFileInfo();
  if(fi == QFileInfo()){
    return ret;
  }
  ImageCategory * ic = ImageCategory::getFileCategory(file);
  if(!ic){
    return ret;
  }
  for(int i = 0;i<outputFiles.size();i++){
    // Files must have the same ending
    if(outputFiles.at(i).suffix() == fi.suffix()){
      // Files must be of the same category
      if(ic->includes(outputFiles.at(i).fileName())){
	// File must be older than current file
	if(outputFiles.at(i).created() < fi.created()){
	  if(ret.fileName().isEmpty()){
	    ret = outputFiles.at(i);
	  }else{
	    if(outputFiles.at(i).created() > ret.created()){
	      ret = outputFiles.at(i);
	    }
	  }
	}
      }
    }  
  }
  return ret;
}


void OutputWatcher::run(){
  pooler->start(2000);
}


QString OutputWatcher::incrementFilename(QString file, int increment){
  QFileInfo fi = QFileInfo(file);
  QString base = fi.baseName();
  QRegExp rep  = QRegExp("(.*)-(\\d+)");
  rep.exactMatch(base);
  int iter = rep.cap(2).toInt();
  iter += increment;
  if(iter < 0){
    return QString();    
  }
  base = rep.cap(1)+"-"+ QString("%1").arg(iter,7,10,QChar('0'));
  QString out= fi.path()+"/"+base+"."+fi.completeSuffix();
  //  qDebug("New filename %s",out.toAscii().data());
  return out;  
}


bool OutputWatcher::isFileValid(QString file){
  QFileInfo fileInfo(file);
  if(!fileInfo.exists()){
    //    qDebug(("File does not exist:" +file).toAscii());
    return false;
  }
  if(fileInfo.created() < initTime){
    //    qDebug(("File is old:" +file).toAscii());
    return false;
  }
  return true;
}
