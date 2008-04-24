#include "mainwindow.h"
#include "server.h"
#include "ui_Hawk.h"
#include "workspace.h"
#include <QtGui>
#include <spimage.h>
#include "applicationmode.h"
#include "imageItem.h"

MainWindow::MainWindow(QMainWindow * parent)
  :QMainWindow(parent)
{
  setWindowIcon(QIcon(":/images/big/HawkIcon.png"));
  setupUi(this);
  QActionGroup *myActionGroup = new QActionGroup(this);
  myActionGroup->addAction(actionPreprocess);
  myActionGroup->addAction(actionReconstruct);
  myActionGroup->addAction(actionAnalyse);
  myActionGroup->setExclusive(true);
  QActionGroup *modeActionGroup = new QActionGroup(this);
  modeActionGroup->addAction(actionExcludeFromMask);
  modeActionGroup->addAction(actionIncludeInMask);
  modeActionGroup->addAction(actionNavigate);
  modeActionGroup->addAction(actionPickCenter);
  modeActionGroup->setExclusive(true);


  actionSaveImage->setEnabled(false);
  connect(this,SIGNAL(selectedImageItemChanged(ImageItem *)),this,SLOT(fillImagePropertiesTable(ImageItem *)));

  workspaces.append(new Workspace(tab,this));
  vboxLayout1->addWidget(workspaces.last()); 
  vboxLayout1->setMargin(4); 
  server = new Server(this,1050);
  selImageItem = NULL;
  
}

void MainWindow::on_actionOpenImage_triggered(bool checked){
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
			       "",
			       tr("Images (*.h5 *.tiff *.tif)"));
  if(filename.isEmpty()){
    return;
  }
  Workspace * ws = workspaces[workspaceTab->currentIndex()];
  ws->loadImage(filename);
}

void MainWindow::on_actionExcludeFromMask_toggled(bool checked){
  if(checked){
    emit modeChanged(ModeExcludeFromMask);
  }
}

void MainWindow::on_actionIncludeInMask_toggled(bool checked){
  if(checked){
    emit modeChanged(ModeIncludeInMask);
  }
}

void MainWindow::on_actionNavigate_toggled(bool checked){
  if(checked){
    emit modeChanged(ModeDefault);
  }
}

void MainWindow::on_actionPickCenter_toggled(bool checked){
  if(checked){
    emit modeChanged(ModePickCenter);
  }
}

void MainWindow::on_actionExit_triggered(bool checked){
  if(checked){
    QApplication::instance()->quit();
  }
}

void MainWindow::on_actionSaveImage_triggered(bool checked){
  QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"),QString(), tr("Images (*.h5;*.vtk;*.png)"));
  if(!filename.isEmpty() && selImageItem){
    if(QFile::exists(filename)){
      // Seems like the hdf5 library is macosx doesn't like to overwrite files
      QFile::remove(filename);
    }

    sp_image_write(selImageItem->getImage(),filename.toAscii(),0);
  }
  return;
}


void MainWindow::setSelectedImageItem(ImageItem * it){
  if(selImageItem != it){
    selImageItem = it;
    emit selectedImageItemChanged(it);
  }
  if(it){
    actionSaveImage->setEnabled(true);
  }else{
    actionSaveImage->setEnabled(false);
  }
}

void MainWindow::imageItemChanged(ImageItem * it){
  if(selImageItem == it){
    fillImagePropertiesTable(it);
  }
}


void MainWindow::fillImagePropertiesTable(ImageItem * it){
  if(it){
    Image * a = it->getImage();
    Workspace * ws = workspaces[workspaceTab->currentIndex()];
    QTableWidget * table = ws->getPropertiesTable();
    QTableWidgetItem * property;
    QTableWidgetItem * value;
    while(table->rowCount()){
      table->removeRow(0);
    }
    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Image Width (px)"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QString::number(sp_image_x(a)));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Image Height (px)"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QString::number(sp_image_y(a)));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);



    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Center X (px)"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setData(Qt::UserRole,QString("real_editable"));
    value->setText(QString::number(a->detector->image_center[0]));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Center Y (px)"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setData(Qt::UserRole,QString("real_editable"));
    value->setText( QString::number((a->detector->image_center[1])));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    QFileInfo fi(it->getFilename());

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("File"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QDir::toNativeSeparators(fi.fileName()));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Directory"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QDir::toNativeSeparators(fi.absolutePath()));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);


    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Detector Distance"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QString::number(a->detector->detector_distance));
    value->setData(Qt::UserRole,QString("positive_real_editable"));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Pixel Width (%1m)").arg(QChar(0x03BC)));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QString::number(a->detector->pixel_size[0]));
    value->setData(Qt::UserRole,QString("positive_real_editable"));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Pixel Height (%1m)").arg(QChar(0x03BC)));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setData(Qt::UserRole,QString("positive_real_editable"));
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setText(QString::number(a->detector->pixel_size[1]));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);


    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Wavelength (nm)"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setData(Qt::UserRole,QString("positive_real_editable"));
    value->setText( QString::number(a->detector->lambda));
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->insertRow(table->rowCount());
    property = new  QTableWidgetItem;
    property->setText(tr("Phased"));
    value = new  QTableWidgetItem;
    value->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    value->setData(Qt::UserRole,QString("bool_editable"));
    property->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    if(a->phased){
      value->setText("true");
    }else{
      value->setText("false");
    }
    table->setItem(table->rowCount()-1,0,property);
    table->setItem(table->rowCount()-1,1,value);

    table->update();
    table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    
    
  }


  
}
