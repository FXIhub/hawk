#include "imageloader.h"

QMutex ImageLoader::loading ;
ImageLoader::ImageLoader(QString file,QObject * parent)
  :QThread(parent)
{
  p_file = file;
}

void ImageLoader::run(){
  loading.lock();
  loadImage(p_file);
  loading.unlock();
}

void ImageLoader::loadImage(QString file){
  qDebug("Loading %s",file.toAscii().data());
  p_image = sp_image_read(file.toAscii(),0);
  if(!p_image){
    qDebug("Loading failed %s",file.toAscii().data());
    // loading failed. Try again 
    usleep(500000);
    qDebug("Retrying %s",file.toAscii().data());
    p_image = sp_image_read(file.toAscii(),0);
  }
}

Image * ImageLoader::getImage(){
  return p_image;
}

QString ImageLoader::getFile(){
  return p_file;
}
