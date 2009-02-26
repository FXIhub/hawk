#include "imageframe.h"
#include "imageview.h"
#include "imagecategory.h"
#include "imagedisplay.h"
#include "outputwatcher.h"
#include "processcontrol.h"
#include <QtGui>

ImageFrame::ImageFrame(ImageView * view, ImageDisplay * parent)
 :QWidget(parent)
{
  imageDisplay = parent;
  imageView = view;
  QVBoxLayout *layout = new QVBoxLayout;
  topLayout = new QHBoxLayout;

  title = new QLabel("title",this);
  title->setAlignment(Qt::AlignHCenter);
  title->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  categoryBox = new QComboBox(this);
  categoryBox->setHidden(true);

  setTabOrder(categoryBox,this);

  connect(categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(onCategoryBoxChanged(int)));

  topLayout->addWidget(title);
  topLayout->addWidget(categoryBox);
  layout->addLayout(topLayout);
  layout->addWidget(imageView);
  setLayout(layout);

  imageLoadingTimer = new QTimer(this);
  imageLoadingTimer->setSingleShot(true);
  connect(imageLoadingTimer,SIGNAL(timeout()),this,SLOT(onImageLoadingTimerTimeout()));
}


void ImageFrame::onImageLoaded(QString file){
  filename = file;
  QFileInfo fi = QFileInfo(file);
  setTitle(fi.fileName());
  QList<QString> items;
  QList<QString> items_data;
  /* Check for categories before touching the combobox as the user might be using it */
  if(fi.exists() && imageCategories){
    ImageCategory * ic = ImageCategory::getFileCategory(file);
    if(ic){
      categoryBox->setHidden(false);    
      int size = imageCategories->size();      
      QString iteration = imageView->getCurrentIteration();
      for(int j = 0;j<size;j++){
	QString trans = fi.absolutePath() + "/" + imageCategories->at(j)->fileNameFromIteration(iteration);
	if(QFileInfo(trans).exists()){
	  items.append(imageCategories->at(j)->getName());
	  items_data.append(trans);
	}
      }
      size = items.size();
      for(int j = 0;j<size;j++){
	int index = categoryBox->findText(items.at(j));
	if(index >= 0){
	  // item already exists. Just change the data
	  categoryBox->setItemData(index,items_data.at(j));
	}else{
	  // item does not exist
	  categoryBox->addItem(items.at(j));
	  categoryBox->setItemData(categoryBox->count()-1,items_data.at(j));
	}
      }
      categoryBox->setCurrentIndex(categoryBox->findText(ic->getName()));
    }else{
      categoryBox->setHidden(true);
      categoryBox->clear();
    }
  }else{
    categoryBox->setHidden(true);
    categoryBox->clear();
  }
}

void ImageFrame::setTitle(QString s){
  title->setText(s);
}

void ImageFrame::setImageCategories(QList<ImageCategory *> * ic){
  imageCategories = ic;
}


void ImageFrame::onCategoryBoxChanged(int index){
  if(index == -1){
    return;
  }
  if(categoryBox->itemData(index).toString().isEmpty()){
    return;
  }
  if(filename != categoryBox->itemData(index)){
    qDebug("Trying to load %s",categoryBox->itemData(index).toString().toAscii().data());
    imageView->loadImage(categoryBox->itemData(index).toString());
  }
}

void ImageFrame::keyPressEvent ( QKeyEvent * event ){
  if(event->key() == Qt::Key_Up){
    QFileInfo fi = getNextFile(filename);
    if(!fi.fileName().isEmpty()){
      setTitle(fi.fileName());
      filename = fi.absoluteFilePath();
      imageLoadingTimer->start(200);
    }      
  }
  if(event->key() == Qt::Key_Down){
    QFileInfo fi = getPreviousFile(filename);
    if(!fi.fileName().isEmpty()){
      setTitle(fi.fileName());
      filename = fi.absoluteFilePath();
      imageLoadingTimer->start(200);
    }      
  }
}


QFileInfo ImageFrame::getNextFile(QString file){
  int increment = 0;
  ProcessControl * pc = imageDisplay->getProcess();
  QFileInfo fi = QFileInfo(file);
  if(pc){
    increment = pc->getOptions()->output_period;
  }else{
    if(suggestedIncrement.contains(fi.path())){
      // There is already an increment for this directory, use that
      increment = suggestedIncrement.value(fi.path());
    }else{
      increment = discoverIncrement(fi);
      suggestedIncrement.insert(fi.path(),increment);
    }
  }  
  file = OutputWatcher::incrementFilename(file,increment);
  QFileInfo ret = QFileInfo(file);
  if(ret.exists() && ret.created() >= fi.created()){
    return ret;
  }
  return QFileInfo();
}

QFileInfo ImageFrame::getPreviousFile(QString file){
  int increment = 0;
  ProcessControl * pc = imageDisplay->getProcess();
  QFileInfo fi = QFileInfo(file);
  if(pc){
    increment = pc->getOptions()->output_period;
  }else{
    if(suggestedIncrement.contains(fi.path())){
      // There is already an increment for this directory, use that
      increment = suggestedIncrement.value(fi.path());
    }else{
      increment = discoverIncrement(fi);
      suggestedIncrement.insert(fi.path(),increment);
    }
  }  
  file = OutputWatcher::incrementFilename(file,-increment);
  QFileInfo ret = QFileInfo(file);
  if(ret.exists() && ret.created() <= fi.created()){
    return ret;
  }
  return QFileInfo();
}

void ImageFrame::onImageLoadingTimerTimeout(){
  qDebug("Loading Image");
  imageView->loadImage(filename);
}



int ImageFrame::discoverIncrement(QFileInfo fi) const{
  // We have no idea of the increment lets try to determine it the hard way
  QDir dir = QDir(fi.absolutePath());
  QStringList filters;
  QRegExp rep  = QRegExp("(.*)-(\\d+)");
  rep.exactMatch(fi.baseName());
  int iter1 = rep.cap(2).toInt();
  QString s = rep.cap(2);
  s.chop(4);
  filters << rep.cap(1)+"-"+s+"*"+fi.completeSuffix();
  dir.setNameFilters(filters);  
  dir.setFilter(QDir::Files|QDir::Readable);
  QStringList currentFiles = dir.entryList();
  int size = currentFiles.size();
  QFileInfo previous;
  QFileInfo next;
  for(int i = 0;i<size;i++){
    QFileInfo fi2 = QFileInfo(dir,currentFiles[i]);
    if(fi2.created() > fi.created()){
      if(next.fileName().isEmpty()){
	next = fi2;
      }else{
	if(fi2.created() < next.created()){
	  next = fi2;
	}
      }
    }else if(fi2.created() < fi.created()){
      if(previous.fileName().isEmpty()){
	previous = fi2;
      }else{
	if(fi2.created() > previous.created()){
	  previous = fi2;
	}
      }    
    }
  }
  int iter0 = -1;
  if(!previous.fileName().isEmpty()){
    rep.exactMatch(previous.baseName());
    iter0 = rep.cap(2).toInt();
  }
  int iter2 = -1;
  if(!next.fileName().isEmpty()){
    rep.exactMatch(next.baseName());
    iter2 = rep.cap(2).toInt();
  }
  if(iter0 == -1 && iter2 == -1){
    qDebug("Could not determine increment");
    return -1;
  }
  if(iter0 == -1){
    return iter2-iter1;
  }
  if(iter2 == -1){
    return iter1-iter0;
  }
  if(iter2-iter1 == iter1-iter0){
    return iter2-iter1;
  }else{
    qDebug("Iteration increment not constant");
    return iter2-iter1;
  }
  return -1;
}



/*
QFileInfo ImageFrame::getNextFile2(QString file) const{
  QFileInfo fi = QFileInfo(file);
  QDir dir = QDir(fi.absolutePath());
  QStringList filters;
  filters << "*.h5";
  filters << "*.png";
  dir.setNameFilters(filters);  
  dir.setFilter(QDir::Files|QDir::Readable);
  //  dir.setSorting(QDir::Time);
  QStringList currentFiles = dir.entryList();
  int size = currentFiles.size();

  ImageCategory * ic = ImageCategory::getFileCategory(file);
  QFileInfo ret;
  if(!ic){
    return ret;
  }
  for(int i = 0;i<size;i++){
    if(currentFiles[i].endsWith(fi.suffix())){
      QFileInfo fi2 = QFileInfo(dir,currentFiles[i]);
      if(fi2.created() > fi.created()){
	if(ic->includes(currentFiles[i])){
	  if(ret.fileName().isEmpty()){
	    ret = fi2;
	  }else{
	    if(fi2.created() < ret.created()){
	      ret = fi2;
	    }
	  }
	}
      }
    }
  }  
  return ret;
}
*/


/*
QFileInfo ImageFrame::getPreviousFile(QString file){
  QFileInfo fi = QFileInfo(file);
  QDir dir = QDir(fi.absolutePath());
  QStringList filters;
  filters << "*.h5";
  filters << "*.png";
  dir.setNameFilters(filters);  
  dir.setFilter(QDir::Files|QDir::Readable);
  //  dir.setSorting(QDir::Time);
  QStringList currentFiles = dir.entryList();
  int size = currentFiles.size();

  ImageCategory * ic = ImageCategory::getFileCategory(file);
  QFileInfo ret;
  if(!ic){
    return ret;
  }
  for(int i = 0;i<size;i++){
    if(currentFiles[i].endsWith(fi.suffix())){
      QFileInfo fi2 = QFileInfo(dir,currentFiles[i]);
      if(fi2.created() < fi.created()){
	if(ic->includes(currentFiles[i])){
	  if(ret.fileName().isEmpty()){
	    ret = fi2;
	  }else{
	    if(fi2.created() > ret.created()){
	      ret = fi2;
	    }
	  }
	}
      }
    }
  }  
  return ret;

}
*/
