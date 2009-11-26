#include <QtGui>
#include "imagedisplay.h"
#include "geometrycontrol.h"
#include "imageview.h"
#include "outputwatcher.h"
#include "imageframe.h"
#include "imagecategory.h"
#include "processcontrol.h"
#include "rpcimageloader.h"

ImageDisplay::ImageDisplay(QWidget * parent)
  :QFrame(parent)
{
  
  geometryControl = new GeometryControl;
  QSize size = geometryControl->getGeometry();
  gridLayout = new QGridLayout;
  setLayout(gridLayout);
  focusFrame = new QFocusFrame(this);
  initImageViewers();
  onFocusedViewChanged(imageViewers[0]);
  lockedBrowse = false;
  outputWatcher = NULL;
  processRunning = false;
  process = NULL;
}


void ImageDisplay::initImageViewers(){
  QSize size = geometryControl->getGeometry();
  for(int x = 0;x<size.width();x++){
    for(int y = 0;y<size.height();y++){
      ImageView * iv = new ImageView(this);
      ImageFrame * frame = new ImageFrame(iv,this);

      connect(iv,SIGNAL(focusedIn(ImageView *)),this,SLOT(onFocusedViewChanged(ImageView *)));
      connect(iv,SIGNAL(focusedIn(ImageView *)),this,SIGNAL(focusedViewChanged(ImageView *)));
      connect(iv,SIGNAL(scaleBy(qreal)),this,SLOT(scaleView(qreal)));
      connect(iv,SIGNAL(translateBy(QPointF)),this,SLOT(translateView(QPointF)));
      connect(iv,SIGNAL(imageLoaded(QString)),frame,SLOT(onImageLoaded(QString)));
      connect(iv,SIGNAL(imageLoaded(QString)),this,SLOT(onImageLoaded(QString)));

      iv->loadImage(QPixmap(":/images/HawkIcon.png"));
      imageViewers.append(iv);
      imageFrames.append(frame);
      imageViewersPos.append(QPoint(x,y));

      gridLayout->addWidget(frame,y,x);
    }
  }
}

void ImageDisplay::onFocusedViewChanged(ImageView * focusedView){
  selected = focusedView;
  focusFrame->setWidget(selected);

}

void ImageDisplay::setLockedTransformation(bool checked){
  locked = checked;
  if(checked){
    // make sure all images display the same
    QPointF pos = selected->getPos();
    QTransform transform = selected->getTransform();
    for(int i = 0;i<imageViewers.size();i++){
      imageViewers[i]->setTransform(transform);
      imageViewers[i]->setPos(pos);
    }
  }
}

void ImageDisplay::scaleView(qreal scale){
  if(locked){
    for(int i = 0;i<imageViewers.size();i++){
      /* don't scale the view that originated the signal */
      if((QObject *)imageViewers[i] != sender()){
	imageViewers[i]->scaleItems(scale);
      }
    }
  }
}

void ImageDisplay::translateView(QPointF t){
  if(locked){
    for(int i = 0;i<imageViewers.size();i++){
      /* don't translate the view that originated the signal */
      if((QObject *)imageViewers[i] != sender()){
	imageViewers[i]->translateItems(t);
      }
    }
  }
}

void ImageDisplay::onProcessStarted(ProcessControl::ProcessType type, QString path,ProcessControl * p){
  outputWatcher = new OutputWatcher(path,this,imageCategories,p->getOptions()->output_period,p->startTime());
  if(type == ProcessControl::Local){
    connect(outputWatcher,SIGNAL(newOutput(QString,QFileInfo,QFileInfo)),this,SLOT(updateLatestOutput(QString,QFileInfo,QFileInfo)));
    connect(outputWatcher,SIGNAL(initialOutput(QString,QFileInfo)),this,SLOT(loadInitialProcessOutput(QString,QFileInfo)));
    outputWatcher->start(QThread::IdlePriority);
  }else if(type == ProcessControl::Embedded){
    connect(outputWatcher,SIGNAL(newOutput(QString,QFileInfo,QFileInfo)),this,SLOT(updateLatestOutput(QString,QFileInfo,QFileInfo)));
    connect(outputWatcher,SIGNAL(initialOutput(QString,QFileInfo)),this,SLOT(loadInitialProcessOutput(QString,QFileInfo)));
    outputWatcher->start(QThread::IdlePriority);
  }else if(type == ProcessControl::NetworkRPC){
    connect(p->rpcImageLoader(),SIGNAL(imageOutputReceived(QString,QFileInfo,QFileInfo)),this,SLOT(updateLatestOutput(QString,QFileInfo,QFileInfo)));
    connect(p->rpcImageLoader(),SIGNAL(initialImageOutputReceived(QString,QFileInfo)),this,SLOT(loadInitialProcessOutput(QString,QFileInfo)));
  }else{
    qWarning("Process type unkown in %s:%d",__FILE__,__LINE__);
  }
  processRunning = true;
  process = p;
}

void ImageDisplay::onProcessStopped(){
  if(outputWatcher){
    connect(outputWatcher,SIGNAL(finished()),this,SLOT(deleteOutputWatcher()));
    outputWatcher->stop();
    //    delete outputWatcher;
  }
  processRunning = false;
}

void ImageDisplay::updateLatestOutput(QString type,QFileInfo file,QFileInfo old){
  //  qDebug("here");
  int size = imageViewers.size();
  for(int i = 0;i<size;i++){
    if(imageViewers[i]->getAutoUpdate() == true){
      QFileInfo fi = QFileInfo(imageViewers[i]->newestFilename());
      //      qDebug(("Checking if " + fi.absoluteFilePath() + " matches "+ old.absoluteFilePath()).toAscii());
      if(fi == old ){
	imageViewers[i]->scheduleImageLoad(file.absoluteFilePath());
      }
    }
  }
}

void ImageDisplay::updateLatestOutput(quint64 client,QString location){
  //  qDebug("ImageDisplay: updateLatestOutput");
  int size = imageViewers.size();
  for(int i = 0;i<size;i++){
    if(imageViewers[i]->getAutoUpdate() == true){
      RPCImageLoader * ril = process->rpcImageLoader();
      qDebug("ImageDisplay: comparing %s to %s",location.toAscii().constData(),imageViewers[i]->newestFilename().toAscii().constData());
      qDebug("ImageDisplay: next in sequence %s",ril->nextInSequence(client,imageViewers[i]->newestFilename()).toAscii().constData());
      if(ril->nextInSequence(client,imageViewers[i]->newestFilename()) == location){
	qDebug("ImageDisplay: trying to load %s",location.toAscii().constData());
	connect(process->rpcImageLoader(),SIGNAL(imageLoaded(quint64, QString,Image *)),this,SLOT(finishLoadRPCImage(quint64, QString,Image *)));
	m_rpcImageToView.insert(QPair<quint64,QString>(client,location),imageViewers[i]);
	ril->loadImage(client,location);
      }
    }
  }
}

void ImageDisplay::setAutoUpdate(bool update){
  int size = imageViewers.size();
  for(int i = 0;i<size;i++){
    imageViewers[i]->setAutoUpdate(update);
  }
}

void ImageDisplay::loadUserSelectedImage(){
  if(selected){
    selected->loadUserSelectedImage();
  }
}

void ImageDisplay::setImageCategories(QList<ImageCategory * > * ic){
  imageCategories = ic;
  for(int i = 0;i<imageFrames.size();i++){
    imageFrames[i]->setImageCategories(imageCategories);
  }

}

OutputWatcher * ImageDisplay::getOutputWatcher(){
  return outputWatcher;
}

bool ImageDisplay::isProcessRunning(){
  return processRunning;
}

void ImageDisplay::loadInitialProcessOutput(QString key, QFileInfo file){
  qDebug("Initial Output Received! %s",file.filePath().toAscii().constData());
  if(file.suffix() == "h5"){
    if(key == "Object"){
      if(imageViewers[0]){
	qDebug("ImageDisplay: loading initial object");
	imageViewers[0]->loadImage(file.absoluteFilePath());
      }
    }
    if(key == "Support"){
      if(imageViewers[1]){
	qDebug("ImageDisplay: loading initial support");
	imageViewers[1]->loadImage(file.absoluteFilePath());
      }
    }
  }
}

void ImageDisplay::loadInitialProcessOutput(quint64 client, QString location){
  QFileInfo file(location);
  qDebug("ImageDisplay: Initial Output Received! %s",location.toAscii().constData());
  if(file.suffix() == "h5"){
    connect(process->rpcImageLoader(),SIGNAL(imageLoaded(quint64, QString,Image *)),this,SLOT(finishLoadRPCImage(quint64, QString,Image *)));
    if(location.contains("real_out-")){
      if(imageViewers[0]){
	m_rpcImageToView.insert(QPair<quint64,QString>(client,location),imageViewers[0]);
	process->rpcImageLoader()->loadImage(client,location);
      }
    }
    if(location.contains("support-")){
      if(imageViewers[1]){
	m_rpcImageToView.insert(QPair<quint64,QString>(client,location),imageViewers[1]);
	process->rpcImageLoader()->loadImage(client,location);
      }
    }
  }
}


ProcessControl * ImageDisplay::getProcess(){
  return process;
}

void ImageDisplay::shiftSelectedImage(){
  if(selected){
    selected->shiftImage();
  }
}

void ImageDisplay::fourierTransformSelectedImage(){
  if(selected){
    selected->fourierTransform();
  }
}

void ImageDisplay::fourierTransformSquaredSelectedImage(){
  if(selected){
    selected->fourierTransformSquared();
  }
}


void ImageDisplay::displayAmplitudes(){
  if(selected){
    selected->setDisplay(0);
  }
}

void ImageDisplay::displayPhases(){
  if(selected){
    selected->setDisplay(SpColormapPhase);
  }
}

void ImageDisplay::displayMask(){
  if(selected){
    selected->setDisplay(SpColormapMask);
  }
}

void ImageDisplay::setColorGray(){
  if(selected){
    selected->setColormap(SpColormapGrayScale);
  }
}


void ImageDisplay::setColorJet(){
  if(selected){
    selected->setColormap(SpColormapJet);
  }
}

void ImageDisplay::setColorHot(){
  if(selected){
    selected->setColormap(SpColormapHot);
  }
}

void ImageDisplay::setColorWheel(){
  if(selected){
    selected->setColormap(SpColormapWheel);
  }
}

void ImageDisplay::setColorTraditional(){
  if(selected){
    selected->setColormap(SpColormapTraditional);
  }
}


void ImageDisplay::setColorRainbow(){
  if(selected){
    selected->setColormap(SpColormapLogScale);
  }
}


void ImageDisplay::setColormap(int color){
  if(selected){
    selected->setColormap(color);
  }
}

void ImageDisplay::setDisplay(int display){
  if(selected){
    selected->setDisplay(display);
  }
}


void ImageDisplay::setLockedBrowse(bool locked){
  lockedBrowse = locked;  
}

void ImageDisplay::onImageLoaded(QString image){
  int size = imageViewers.size();
  QString iteration = ImageCategory::getFileIteration(image);
  if(iteration.isEmpty()){
    return;
  }
  if(lockedBrowse){
    for(int i = 0;i<size;i++){
      QString file = imageViewers.at(i)->getFilename();    
      QString newFile = ImageCategory::setFileIteration(file,iteration);
      if(newFile.isEmpty() || newFile == file){
	continue;
      }
      if(QFileInfo(newFile).exists()){
	imageViewers.at(i)->loadImage(newFile);    
      }
    }
  }
}

void ImageDisplay::maxContrastSelectedImage(){
  if(selected){
    selected->maxContrast();
  }
}

void ImageDisplay::logScaleSelectedImage(bool on){
  if(selected){
    selected->setLogScale(on);
  }
}

void ImageDisplay::deleteOutputWatcher(){
  delete sender();
}

void ImageDisplay::finishLoadRPCImage(quint64 client, QString location,Image * a){
  if(m_rpcImageToView.contains(QPair<quint64,QString>(client,location))){
    ImageView * v = m_rpcImageToView.take(QPair<quint64,QString>(client,location));
    v->loadImageFromMemory(a,location);
    qDebug("ImageDisplay: Finished loading %s",location.toAscii().constData());
  }
}
