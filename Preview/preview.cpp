#include "preview.h"
#include "ui_preview.h"
#include <spimage.h>
#include <QFileDialog>
#include <QStringListModel>
#include <iostream>

Preview::Preview(QMainWindow * parent)
  :QMainWindow(parent)
{
  img = 0;
  colormap_data = 0;
  setupUi(this);
  imageRange->setMaximum(65535);
  imageRange->setMinimum(0);
  imageRange->setValue(65535);
}

void Preview::on_actionOpen_Directory_triggered(bool checked){
 QString dirname = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
						     | QFileDialog::DontResolveSymlinks);
 if(dirname.isEmpty()){
   return;
 }
 dir = QDir(dirname);
 QStringList filters;
 filters << "*.tif" << "*.TIF" << "*.TIFF" << "*.tiff";
 dir.setNameFilters(filters);
 QStringList filenames = dir.entryList(QDir::Files);
 filesList->clear();
 filesList->addItems(filenames);
 if(!filenames.isEmpty()){
   filesList->setCurrentRow(0);
 }
}

void Preview::on_filesList_itemSelectionChanged(){
  QList<QListWidgetItem *> selected = filesList->selectedItems();
  if(!selected.empty()){
    QString file = selected.first()->text();
    openImage(file);
  }
}

void Preview::openImage(QString filename){
  if(img){
    sp_image_free(img);
  }
  char * file = dir.absoluteFilePath(filename).toAscii().data();
  img = sp_image_read(file,0);
  if(!img){
    return;
  }    
  loadImageComment(QString(file));
  if(resetRangeCheckBox->checkState() == Qt::Checked){
    imageRange->setValue(65535);
  }
  imageRange->setMaximum(65535);
  imageRange->setMinimum(1);
  on_imageRange_sliderReleased();
}

void Preview::on_logScaleCheckBox_stateChanged(int state){
  on_imageRange_sliderReleased();
}

void Preview::on_imageRange_sliderReleased(){
  int value = imageRange->value();
  int flags;
  if(img){
    free(colormap_data);
    if(logScaleCheckBox->checkState() == Qt::Checked){
      flags = COLOR_JET|LOG_SCALE;
    }else{
      flags = COLOR_JET;
    }
    colormap_data = sp_image_get_false_color(img,flags,-1,value);

    QImage qi = QImage(colormap_data,sp_image_x(img),sp_image_y(img),QImage::Format_RGB32);
    if(sp_image_y(img) > 512){
      imageLabel->setPixmap(QPixmap::fromImage(qi).scaledToHeight(512)); 
    }else{
      imageLabel->setPixmap(QPixmap::fromImage(qi)); 
    }
  }
}

void Preview::loadNextImage(){
  QList<QListWidgetItem *> selected = filesList->selectedItems();    
  if(selected.empty()){
    return;
  }
  if(filesList->currentRow()+1 == filesList->count()){
    return;
  }
  filesList->setCurrentRow(filesList->currentRow()+1);
}

void Preview::loadImageComment(QString filename){
  filename.truncate(filename.size()-4);
  filename.append(".txt");
  QFile file(filename);
  if(!file.exists()){
    return;
  }
  if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
    return;
  }
  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    if(line.contains("Master_comment")){
      commentBrowser->setText(line);
      break;
    }
  }
  QString imgsize = QString("Image size: ");
  imgsize.append(QString::number(sp_image_x(img)));
  imgsize.append("x");
  imgsize.append(QString::number(sp_image_y(img)));
  imgsize.append("\n");
  commentBrowser->append(imgsize);

}

void Preview::on_actionCopyHit_triggered(bool checked){
  QList<QListWidgetItem *> selected = filesList->selectedItems();    
  if(selected.empty()){
    return;
  }
  QString sel = selected.first()->text();
  QString file = dir.absoluteFilePath(sel);
  if(!dir.exists("Hits")){
    dir.mkdir("Hits");
  }
  QDir destdir = QDir(dir);
  if(!destdir.cd("Hits")){
    printf("Canot changed to dir\n");
  }
  QString dest = destdir.absoluteFilePath(sel);
  QFile::copy(file,dest);    
  /* Also copy comment */
  file.truncate(file.size()-4);
  file.append(".txt");
  dest.truncate(dest.size()-4);
  dest.append(".txt");
  if(dir.exists(file)){
    QFile::copy(file,dest);    
  }

}
