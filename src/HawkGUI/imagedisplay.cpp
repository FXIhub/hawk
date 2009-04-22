#include <QtGui>
#include "imagedisplay.h"
#include "geometrycontrol.h"
#include "imageview.h"
#include "outputwatcher.h"
#include "imageframe.h"
#include "imagecategory.h"
#include "processcontrol.h"

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
      imageViewers[i]->scaleItems(scale);
    }
  }else{
    if(selected){
      selected->scaleItems(scale);
    }
  }
}

void ImageDisplay::translateView(QPointF t){
  if(locked){
    for(int i = 0;i<imageViewers.size();i++){
      imageViewers[i]->translateItems(t);
    }
  }else{
    if(selected){
      selected->translateItems(t);
    }
  }
}

void ImageDisplay::onProcessStarted(QString type, QString path,ProcessControl * p){
  outputWatcher = new OutputWatcher(path,this,imageCategories,p->getOptions()->output_period,p->startTime());
  if(type == QString("local")){
    connect(outputWatcher,SIGNAL(newOutput(QString,QFileInfo,QFileInfo)),this,SLOT(updateLatestOutput(QString,QFileInfo,QFileInfo)));
    //    qDebug("connecting");
    connect(outputWatcher,SIGNAL(initialOutput(QString,QFileInfo)),this,SLOT(loadInitialProcessOutput(QString,QFileInfo)));
    outputWatcher->start(QThread::IdlePriority);
  }else if(type == QString("embedded")){
    connect(outputWatcher,SIGNAL(newOutput(QString,QFileInfo,QFileInfo)),this,SLOT(updateLatestOutput(QString,QFileInfo,QFileInfo)));
    //    qDebug("connecting");
    connect(outputWatcher,SIGNAL(initialOutput(QString,QFileInfo)),this,SLOT(loadInitialProcessOutput(QString,QFileInfo)));
    outputWatcher->start(QThread::IdlePriority);
  }else{
    qDebug("Process type unkown in %s:%d",__FILE__,__LINE__);
  }
  connect(this,SIGNAL(stopOutputWatcher()),outputWatcher,SLOT(stop()));
  processRunning = true;
  process = p;
}

void ImageDisplay::onProcessStopped(){
  if(outputWatcher){
    emit stopOutputWatcher();
    outputWatcher = NULL;
    /*    outputWatcher->stop();
	  delete outputWatcher;*/
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

void ImageDisplay::setAutoUpdate(bool update){
  int size = imageViewers.size();
  for(int i = 0;i<size;i++){
    imageViewers[i]->setAutoUpdate(update);
  }
}

void ImageDisplay::loadUserSelectedImage(){
  QString fileName = QFileDialog::getOpenFileName(this, tr("Load Image"),
						  QString(),
						  tr("Images (*.h5 *.png *tif *tiff)"));
  if(!fileName.isEmpty()){
    selected->loadImage(fileName);
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
  qDebug("Initial Output Received!");
  if(file.suffix() == "h5"){
    if(key == "Object"){
      if(imageViewers[0]){
	imageViewers[0]->loadImage(file.absoluteFilePath());
      }
    }
    if(key == "Support"){
      if(imageViewers[1]){
	imageViewers[1]->loadImage(file.absoluteFilePath());
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

void ImageDisplay::displayAmplitudes(){
  if(selected){
    selected->setDisplay(0);
  }
}

void ImageDisplay::displayPhases(){
  if(selected){
    selected->setDisplay(COLOR_PHASE);
  }
}

void ImageDisplay::displayMask(){
  if(selected){
    selected->setDisplay(COLOR_MASK);
  }
}

void ImageDisplay::setColorGray(){
  if(selected){
    selected->setColormap(COLOR_GRAYSCALE);
  }
}


void ImageDisplay::setColorJet(){
  if(selected){
    selected->setColormap(COLOR_JET);
  }
}

void ImageDisplay::setColorHot(){
  if(selected){
    selected->setColormap(COLOR_HOT);
  }
}

void ImageDisplay::setColorWheel(){
  if(selected){
    selected->setColormap(COLOR_WHEEL);
  }
}

void ImageDisplay::setColorTraditional(){
  if(selected){
    selected->setColormap(COLOR_TRADITIONAL);
  }
}


void ImageDisplay::setColorRainbow(){
  if(selected){
    selected->setColormap(COLOR_RAINBOW);
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
    selected->logScale(on);
  }
}
