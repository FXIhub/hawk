#include "imagecategory.h"

QList<ImageCategory *> ImageCategory::categories;

ImageCategory::ImageCategory(QString n, QString id, bool iterationDependent){
  identifier = id;
  iterationSuffix = iterationDependent;
  if(iterationSuffix){
    regExp = QRegExp(".*/*"+ identifier + "-(\\d+)\\.h5");  
  }else{
    regExp = QRegExp(".*/*"+ identifier + "\\.h5");  
  }
  name = n;
  categories.append(this);
}

bool ImageCategory::includes(const QString filename) const{
  return regExp.exactMatch(filename);
}

QString ImageCategory::getName() const{
  return name;
}

QString ImageCategory::getIdentifier() const{
  return identifier;
}

QString ImageCategory::translateFilename(const QString filename, const ImageCategory * from, const ImageCategory * to){
  QString ret = filename;
  ret.replace(from->getIdentifier(),to->getIdentifier());
  return ret;
}

ImageCategory * ImageCategory::getFileCategory(const QString filename){
  int size = categories.size();
  for(int i = 0;i<size;i++){
    if(categories.at(i)->includes(filename)){
      return categories.at(i);
    }
  }
  return NULL;
}

QString ImageCategory::getFileIteration(const QString filename){
  int size = categories.size();
  for(int i = 0;i<size;i++){
    if(categories.at(i)->includes(filename)){
      return categories.at(i)->getIteration(filename);
    }
  }
  return QString();
}

QString ImageCategory::setFileIteration(const QString filename,QString iteration){
  int size = categories.size();
  for(int i = 0;i<size;i++){
    if(categories.at(i)->includes(filename)){
      QString from = categories.at(i)->getIteration(filename);
      QString ret = filename;
      ret.replace(from,iteration);
      return ret;
    }
  }
  return QString();
}

QString ImageCategory::getIteration(QString file){
  if(iterationSuffix){
    if(regExp.exactMatch(file)){
      return regExp.cap(1);
    }
  }
  return QString();
}

QString ImageCategory::fileNameFromIteration(QString iteration){
  if(iterationSuffix){
    return identifier+"-"+iteration+".h5";  
  }
  return identifier+".h5";
}
