#include <QtGui>
#include <spimage.h>
#include "imageview.h"
#include "propertiesDialog.h"
#include "cropDialog.h"
#include "backgroundSlider.h"
//#include "look.moc"
#include "look.h"

Look::Look(QWidget *parent)// : QWidget(parent)
{
  //imageLabel = new QLabel;
  view = new ImageView;
  
  filesTable = new QTableWidget(0,3);
  filesTable->setHorizontalHeaderLabels(QStringList() << "File" << "Flag" << "Comment");
  filesTable->setColumnWidth(0,100);
  filesTable->setColumnWidth(1,75);
  filesTable->setColumnWidth(2,150);
  filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  filesTable->setSelectionMode(QAbstractItemView::SingleSelection);
  imagesTable = new QTableWidget(0,3);
  imagesTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Background" << "Remark");
  imagesTable->setColumnWidth(0,150);
  imagesTable->setColumnWidth(0,150);
  imagesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  imagesTable->setSelectionMode(QAbstractItemView::SingleSelection);
  rangeSlider = new QSlider(Qt::Vertical);
  rangeSlider->setRange(0,100);
  rangeSlider->setValue(100);
  comment = new QTextBrowser(this);

  leftTab = new QTabWidget();
  leftTab->addTab(filesTable,"Directory");
  leftTab->addTab(imagesTable,"Images");

  temporary = NULL;
  //backgroundSliderWindow = NULL;
  backgroundWindow = NULL;
  colormap_data = NULL;
  img = NULL;
  draw = NULL;
  background = NULL;
  imgFromList = false;
  drawAuto = 0;
  showMaskActive = false;
  logScale = 0;
  subtract = 0;
  range = 1;
  colorscale = COLOR_JET;
  propertiesSet = false;
  showDistanceActive = false;
  backgroundLevel = NULL;
  centerX = NULL;
  centerY = NULL;
  centerDefined = NULL;
  beamstopX = NULL;
  beamstopY = NULL;
  beamstopR = NULL;
  beamstopDefined = NULL;
  backgroundSlider = NULL;
  pencilSize = 3;
  connect(filesTable, SIGNAL(itemSelectionChanged()), this, SLOT(openImageFromTable()));
  connect(imagesTable, SIGNAL(itemSelectionChanged()), this, SLOT(openImageFromList()));
  connect(rangeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeRange(int)));
  connect(imagesTable, SIGNAL(cellChanged(int,int)), this, SLOT(imagesTableChanged(int,int)));

  connect(view, SIGNAL(centerChanged()), this, SLOT(updateCenter()));
  connect(view, SIGNAL(beamstopChanged()), this, SLOT(updateBeamstop()));
  connect(view, SIGNAL(vertLineSet(real,real)), this, SLOT(vertLineToMaskSlot(real,real)));
  connect(view, SIGNAL(drawMaskAt(real,real)), this, SLOT(drawMaskSlot(real,real)));
  connect(view, SIGNAL(undrawMaskAt(real,real)), this, SLOT(undrawMaskSlot(real,real)));

  filesTable->setMinimumWidth(400);
  //filesTable->setMaximumWidth(300);
  comment->setMinimumWidth(400);
  //comment->setMaximumWidth(300);
  comment->setMinimumHeight(120);
  comment->setMaximumHeight(120);
  //imageLabel->setMinimumWidth(500);
  //imageLabel->setMinimumHeight(500);
  view->setMinimumWidth(500);
  view->setMinimumHeight(500);
  rangeSlider->setMaximumWidth(40);


  QGridLayout *layout = new QGridLayout;
  //layout->addWidget(imageLabel, 0, 1);
  layout->addWidget(view, 0, 1);
  layout->addWidget(comment, 1, 0);
  //layout->addWidget(filesTable,0,0);
  layout->addWidget(leftTab,0,0);
  layout->addWidget(rangeSlider,0,2);

  layout->setColumnStretch(1,2);

  setLayout(layout);
  
}

void Look::openDirectory()
{
  QString dir = QFileDialog::getExistingDirectory(this);
  if(dir.isEmpty()){
    QMessageBox::information(this, tr("Look"), tr("The directory is empty"), QMessageBox::Ok);
    return;
  }
  currentDir = QDir(dir);
  QStringList filters;
  filters << "*.tif" << "*.TIF" << "*.TIFF" << "*.tiff";
  currentDir.setNameFilters(filters);
  QStringList filenames = currentDir.entryList(QDir::Files);
  noOfImages = filenames.size();

  if (noOfImages == 0) {
    QMessageBox::information(this, tr("Look"), tr("There are no TIFF files in thiss directory"), QMessageBox::Ok);
  }

  filesTable->setRangeSelected(QTableWidgetSelectionRange(0,0,filesTable->rowCount()-1,1),false);
  filesTable->setRowCount(noOfImages);

  QProgressDialog progress("Reading directory ...", "Abort", 0, noOfImages, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setValue(0);
  progress.show();

  for (int i = 0; i < noOfImages; i++) {
    if (i%10 == 0) progress.setValue(i);
    QTableWidgetItem *newItem1 = new QTableWidgetItem(filenames.at(i));
    //newItem1->setSizeHint(QSize(50,15));
    newItem1->setFlags(newItem1->flags() & (~Qt::ItemIsEditable));
    filesTable->setItem(i,0,newItem1);
    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    //newItem2->setSizeHint(QSize(50,15));
    newItem2->setFlags(newItem2->flags() & (~Qt::ItemIsEditable));
    filesTable->setItem(i,1,newItem2);
    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    newItem3->setFlags(newItem3->flags() & (~Qt::ItemIsEditable));
    filesTable->setItem(i,2,newItem3);
    if (progress.wasCanceled()) {noOfImages = i; break;}
  }
  filesTable->resizeRowsToContents();
  if (backgroundLevel != NULL) free(backgroundLevel);
  if (centerX != NULL) free(centerX);
  if (centerY != NULL) free(centerY);
  if (centerDefined != NULL) free(centerDefined);
  if (beamstopX != NULL) free(beamstopX);
  if (beamstopY != NULL) free(beamstopY);
  if (beamstopR != NULL) free(beamstopR);
  if (beamstopDefined != NULL) free(beamstopDefined);
  backgroundLevel = (real *) malloc(noOfImages*sizeof(real));
  centerX = (real *) malloc(noOfImages*sizeof(real));
  centerY = (real *) malloc(noOfImages*sizeof(real));
  centerDefined = (bool *) malloc(noOfImages*sizeof(bool));
  beamstopX = (real *) malloc(noOfImages*sizeof(real));
  beamstopY = (real *) malloc(noOfImages*sizeof(real));
  beamstopR = (real *) malloc(noOfImages*sizeof(real));
  beamstopDefined = (bool *) malloc(noOfImages*sizeof(bool));
  for (int i = 0; i < noOfImages; i++) {
    centerDefined[i] = false;
    beamstopDefined[i] = false;
  }
}

void Look::openImageFromTable()
{
  QList<QTableWidgetItem *> selected = filesTable->selectedItems();
  if(!selected.empty()) {
    imagesTable->setRangeSelected(QTableWidgetSelectionRange(0,0,imagesTable->rowCount()-1,1),false);
    int i = filesTable->row(selected.first());
    current = i;
    QString file = filesTable->item(i-1,3)->text();
    openImage(file);

    if (backgroundLevel[current] <= 0.001) {
      backgroundLevel[current] = 1.0;
    }
    if (backgroundSlider) backgroundSlider->setValue(backgroundLevel[current]);

    //update menus
    
    if (filesTable->item(i,1)->text() == "Background") {
      emit backgroundChecked(true);
    } else {
      emit backgroundChecked(false);
    }
    if (filesTable->item(i,1)->text() == "Hit") {
      emit hitChecked(true);
    } else {
      emit hitChecked(false);
    }

    if (centerDefined[current]) view->setCenter(centerX[current],centerY[current]);
    else if (view->setCenterActivated()) {
      centerDefined[current] = true;
      updateCenter();
    }
    if (beamstopDefined[current]) view->setBeamstop(beamstopX[current],beamstopY[current],beamstopR[current]);
    else if (view->defineBeamstopActivated()) {
      beamstopDefined[current] = true;
      updateBeamstop();
    }
    drawImage();
  }
}

void Look::openImageFromList()
{
  QList<QTableWidgetItem *> selected = imagesTable->selectedItems();
  if (!selected.empty()) {
    filesTable->setRangeSelected(QTableWidgetSelectionRange(0,0,filesTable->rowCount()-1,1),false);
    int i = imagesTable->row(selected.first());
    currentImg = i;
    if (img != NULL && !imgFromList) sp_image_free(img);
    img = images.at(i);
    if (!imgFromList) {imgFromList = true; emit imgFromListChanged(true);}

    if (imgCenterDefined[currentImg]) view->setCenter(imgCenterX[currentImg],imgCenterY[currentImg]);
    else if (view->setCenterActivated()) {
      imgCenterDefined[currentImg] = true;
      updateCenter();
    }
    if (imgBeamstopDefined[currentImg]) view->setBeamstop(imgBeamstopX[currentImg],imgBeamstopY[currentImg],imgBeamstopR[currentImg]);
    else if (view->defineBeamstopActivated()) {
      imgBeamstopDefined[currentImg] = true;
      updateBeamstop();
    }
    drawImage();
  }
}

void Look::changeRange(int value)
{
  range = (real) value / 100.0;
  drawImage();
}

void Look::changeBackgroundRange(int value)
{
  backgroundRange = (real) value / 100.0;
  drawImage();
}

void Look::updateCenter()
{
  if (imgFromList) view->getCenter(&imgCenterX[currentImg],&imgCenterY[currentImg]);
  else view->getCenter(&centerX[current],&centerY[current]);
}

void Look::updateBeamstop()
{
  if (imgFromList) view->getBeamstop(&imgBeamstopX[currentImg],&imgBeamstopY[currentImg],&imgBeamstopR[currentImg]);
  else view->getBeamstop(&beamstopX[current],&beamstopY[current],&beamstopR[current]);
}


void Look::openImage(QString filename)
{
  if(img != NULL && !imgFromList){
    sp_image_free(img);
  }
  //QByteArray file = dir.absoluteFilePath(filename).toAscii();
  QByteArray file = currentDir.absoluteFilePath(filename).toAscii();
  if (!useSize) img = sp_image_read(file,0);
  else {
    Image *tmp = sp_image_read(file,0);
    img = bilinear_rescale(tmp, sizeX, sizeY, 1);
    sp_image_free(tmp);
    //img = bilinear_rescale(sp_image_read(file,0), sizeX, sizeY, 1);
  }
  if (imgFromList) {imgFromList = false; emit imgFromListChanged(false);}
  if(!img){
    return;
  }
  drawImage();
  loadImageComment(file);
  /*
  loadImageComment(QString(file));
  if(resetRangeCheckBox->checkState() == Qt::Checked){
    imageRange->setValue(65535);
  }
  imageRange->setMaximum(65535);
  imageRange->setMinimum(1);
  on_imageRange_sliderReleased();
  */
}

void Look::loadImageComment(QString filename)
{
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
      comment->setText(line);
      break;
    }
  }
  QString imgsize = QString("Image size: ");
  imgsize.append(QString::number(sp_image_x(img)));
  imgsize.append("x");
  imgsize.append(QString::number(sp_image_y(img)));
  imgsize.append("\n");
  comment->append(imgsize);

}

void Look::loadAllComments()
{
  int i;
  QString tmp;
  //QFile file;
  QByteArray line;
  QTableWidgetItem *newItem;
  QProgressDialog progress("Reading comments ...", "Abort", 0, filesTable->rowCount(), this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setValue(0);
  progress.show();
  for (i = 0; i < filesTable->rowCount(); i++) {
    if (i%10 == 0) progress.setValue(i);
    tmp = filesTable->item(i-1,3)->text();
    tmp.truncate(tmp.size()-4);
    tmp.append(".txt");
    //file = new QFile
    tmp = currentDir.absoluteFilePath(tmp).toAscii();
    //file = new QFile(tmp);
    QFile file(tmp);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      while (!file.atEnd()) {
	line = file.readLine();
	if(line.contains("Master_comment")){
	  line = line.right(line.length()-17);
	  line.truncate(line.length() - 1);
	  newItem = new QTableWidgetItem((QString) line);
	  newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));    
	  filesTable->setItem(i,2,newItem);
	}
      }
    }
    if(progress.wasCanceled()) break;
  }
}

void Look::updateImagesTable()
{
  imagesTable->setRangeSelected(QTableWidgetSelectionRange(0,0,filesTable->rowCount()-1,1),false);
  imagesTable->setRowCount(images.size());
  QTableWidgetItem *newItem;
  for (int i = 0; i < images.size(); i++) {
    imagesTable->blockSignals(true);
    newItem = new QTableWidgetItem(imageNames.at(i));
    //newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
    imagesTable->setItem(i,0,newItem);
    newItem = new QTableWidgetItem(imgRemark.at(i));
    imagesTable->setItem(i,2,newItem);
    printf("get item\n");
    char buffer[4];
    if (imgBackground[i] > 0) sprintf(buffer,"%i",imgBackground[i]+1);
    else sprintf(buffer,"");
    newItem = new QTableWidgetItem(buffer);
    printf("got item\n");
    imagesTable->setItem(i,1,newItem);
    printf("inserted new item\n");
    imagesTable->blockSignals(false);
  }
}


void Look::drawImage()
{
  if(img != NULL){
    if (colormap_data != NULL) { // &colormap_data before
      //printf("old colormap exists\n");
      free(colormap_data);
      //printf("freed it\n");
    }

    if (subtract) {
      /*
      if (temporary == NULL) temporary = sp_image_alloc(sp_image_x(background), sp_image_y(background), 1);
      else if (sp_image_x(temporary) != sp_image_x(background) || sp_image_y(temporary) != sp_image_y(background))
	sp_image_realloc(temporary,sp_image_x(background),sp_image_y(background),1);
      */
      if (temporary != NULL) sp_image_free(temporary);
      //real sum = 0;
      //for (int i = 0; i < sp_image_size(img); i++) sum += sp_real(img->image->data[i]);
      draw = bilinear_rescale(img,sp_image_x(background),sp_image_y(background),1);
      temporary = sp_image_duplicate(background,SP_COPY_DATA);
      //sp_image_scale(temporary,sum/backgroundSum*backgroundLevel[current]);
      sp_image_scale(temporary,backgroundLevel[current]);
      sp_image_sub(draw,temporary);
    } else { 
      draw = img;
    }

    if (imgFromList && showMaskActive) {
      if (draw == img) draw = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
      for (int i = 0; i < sp_image_size(img); i++) {
	if (draw->mask->data[i] == 0) draw->image->data[i] = sp_cinit(0.0,0.0);
      }
    }
    
    int flags;
    if (logScale) flags = colorscale | LOG_SCALE;
    else flags = colorscale;

    if (drawAuto) {
      if (temporary != NULL) sp_image_free(temporary);
      temporary = sp_image_patterson(draw);
      colormap_data = sp_image_get_false_color(temporary, flags, -1, (int)(65535.0*range*500.0));
    } else {
      colormap_data = sp_image_get_false_color(draw, flags, -1, (int)(65535.0*range));
    }
    
    qi = QImage(colormap_data,sp_image_x(draw),sp_image_y(draw),QImage::Format_RGB32);
    //qi = qi.scaled(imageLabel->width(),imageLabel->height(),Qt::KeepAspectRatio);
    //imageLabel->setPixmap(QPixmap::fromImage(qi));
    view->setImage(&qi);
    if (subtract || (imgFromList && showMaskActive)) sp_image_free(draw);
  }
}

void Look::exportImage()
{
  if(img != NULL){
    QFileDialog saveDialog(this);
    saveDialog.setDefaultSuffix("h5");
    saveDialog.setFileMode(QFileDialog::AnyFile);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    QString filename;
    if (!imgFromList) {
      filename = filesTable->item(current-1,3)->text();
      if (filename.endsWith("tiff",Qt::CaseInsensitive)) filename.chop(4);
      else if (filename.endsWith("tif",Qt::CaseInsensitive)) filename.chop(3);
      else return;
      filename.append("h5");
    } else {
      filename = imageNames[currentImg];
    }
    saveDialog.selectFile(filename);
    saveDialog.setConfirmOverwrite(true);
    saveDialog.setFilters(QStringList() << "HDF5 (*.h5)" << "TIFF image (*.TIFF)" << "png image (*.png)" << "Visit image format (*.vtk)" << "Any files (*)");
    QString outFile;
    if (saveDialog.exec()) outFile = saveDialog.selectedFiles().first();
    else return;
    //saveDialog.setFileMode(QFileDialog::AnyFile);
    //QString outFile = saveDialog.selectedFiles().first();
    //QString outFile = QFileDialog::getSaveFileName(this);
    Image * out;
    if (subtract) {
      /*
      if (temporary == NULL) temporary = sp_image_alloc(sp_image_x(background), sp_image_y(background), 1);
      else if (sp_image_x(temporary) != sp_image_x(background) || sp_image_y(temporary) != sp_image_y(background))
	sp_image_realloc(temporary,sp_image_x(background),sp_image_y(background),1);
      */
      if (temporary != NULL) sp_image_free(temporary);
      temporary = bilinear_rescale(background,sp_image_x(background),sp_image_y(background),1);
      out = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
      //sp_image_scale(temporary,sum/backgroundSum*backgroundLevel[current]);
      sp_image_scale(temporary,backgroundLevel[current]);
      sp_image_sub(out,temporary);
    } else { 
      out = img;
    }
    if (propertiesSet) {
      out->detector->lambda = wavelength;
      out->detector->detector_distance = detectorDistance;
      out->detector->pixel_size[0] = detectorX / (real) sp_image_x(out);
      out->detector->pixel_size[1] = detectorY / (real) sp_image_y(out);
      out->detector->pixel_size[2] = 0;
    }
    if (imgFromList) {
      if (imgCenterDefined[currentImg]) {
	out->detector->image_center[0] = centerX[current] * (real) sp_image_x(out);
	out->detector->image_center[1] = centerY[current] * (real) sp_image_y(out);
	out->detector->image_center[2] = 0.0;
      }
    } else {
      if (centerDefined[current]) {
	out->detector->image_center[0] = centerX[current] * (real) sp_image_x(out);
	out->detector->image_center[1] = centerY[current] * (real) sp_image_y(out);
	out->detector->image_center[2] = 0.0;
      }
    }
    sp_image_write(out,outFile.toAscii(),0);
    
    if (subtract) sp_image_free(out);
  }
}

void Look::exportFalseColorImage()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save false color image"), tr("FalseColor.png"));
  qi.save(filename, "PNG", 100);
}

void Look::exportBackground()
{
  if(background != NULL) {
    QString outFile = QFileDialog::getSaveFileName(this);
    if (propertiesSet) {
      background->detector->lambda = wavelength;
      background->detector->detector_distance = detectorDistance;
      background->detector->pixel_size[0] = detectorX / (real) sp_image_x(background);
      background->detector->pixel_size[1] = detectorY / (real) sp_image_y(background);
      background->detector->pixel_size[2] = 0;
    }
    sp_image_write(background,outFile.toAscii(),0);
  }
}

void Look::exportToDir()
{
  QString dir = QFileDialog::getExistingDirectory(this);
  if(dir.isEmpty()){
    return;
  }
  QDir exportDir = QDir(dir);

  Image *tmp;
  int i;
  int count = 0;
  real sum;
  for (i = 0; i < filesTable->rowCount(); i++)
    if (filesTable->item(i,2)->text() == "Background")
      count++;
  QProgressDialog progress("Exporting ...", "Abort", 0, count, this);
  progress.setWindowModality(Qt::WindowModal);
  count = 0;
  for (i = 0; i < filesTable->rowCount(); i++) {
    if (filesTable->item(i,2)->text() == "Hit") {
      progress.setValue(count);
      if (progress.wasCanceled()) return;
      count++;
      sum = 0;
      tmp = sp_image_read(currentDir.absoluteFilePath(filesTable->item(i-1,3)->text()).toAscii(),0);

      if (propertiesSet) {
	tmp->detector->lambda = wavelength;
	tmp->detector->detector_distance = detectorDistance;
	tmp->detector->pixel_size[0] = detectorX / (real) sp_image_x(tmp);
	tmp->detector->pixel_size[1] = detectorY / (real) sp_image_y(tmp);
	tmp->detector->pixel_size[2] = 0;
      }
      if (centerDefined[i]) {
	tmp->detector->image_center[0] = centerX[i] * (real) sp_image_x(tmp);
	tmp->detector->image_center[1] = centerY[i] * (real) sp_image_y(tmp);
	tmp->detector->image_center[2] = 0.0;
      }

      QString out = exportDir.absolutePath();
      out.append("/");
      out.append(filesTable->item(i-1,3)->text());
      out.chop(3);
      out.append("h5");
      sp_image_write(tmp,out.toAscii(),0);
      //sp_image_write(tmp,exportDir.absoluteFilePath(filesTable->item(i-1,2)->text().chop(3).append("h5")).toAscii(),0);
      //sp_image_write(tmp,exportDir.absolutePath(out).toAscii(),0);
      if (progress.wasCanceled()) return;
      sp_image_free(tmp);
    }
  }
}

void Look::setProperties()
{
  PropertiesDialog *properties = new PropertiesDialog(&wavelength,&detectorDistance, &detectorX, &detectorY, propertiesSet);
  propertiesSet = true;
  properties->exec();
}

void Look::toImages()
{
  if (img != NULL && !imgFromList) {
    if (temporary != NULL) sp_image_free(temporary);
    QString filename = filesTable->item(current-1,3)->text();
    if (useSize && QMessageBox::question(this, tr("Image size changed"), tr("Image size changed, use original size?"),
					 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
      temporary = sp_image_read(currentDir.absoluteFilePath(filename).toAscii(),0);
    } else {
      temporary = sp_image_duplicate(img, SP_COPY_DATA);
    }
    for (int i = 0; i < sp_image_size(temporary); i++) temporary->mask->data[i] = 1;
    images.append(temporary);
    temporary = NULL;


    if (filename.endsWith("tiff",Qt::CaseInsensitive)) filename.chop(5);
    else if (filename.endsWith("tif",Qt::CaseInsensitive)) filename.chop(4);
    else return;
    imageNames.append(filename);

    imgCenterDefined.append(centerDefined[current]);
    imgCenterX.append(centerX[current]); imgCenterY.append(centerY[current]);
    imgBeamstopDefined.append(beamstopDefined[current]);
    imgBeamstopX.append(beamstopX[current]); imgBeamstopY.append(beamstopY[current]);
    imgBeamstopR.append(beamstopR[current]);
    imgRemark.append("");
    imgBackground.append(NULL);
    imgBackgroundLevel.append(1.0);

    updateImagesTable();
  }
}

void Look::backgroundToImages()
{
  if (background != NULL) {
    if (temporary != NULL) sp_image_free(temporary);
    QString name = "background";
    temporary = sp_image_duplicate(img,SP_COPY_DATA);
    for (int i = 0; i < sp_image_size(temporary); i++) temporary->mask->data[i] = 1;
    images.append(temporary);
    temporary = NULL;

    imageNames.append(name);
    imgCenterDefined.append(false);
    imgCenterX.append(centerX[current]); imgCenterY.append(centerY[current]);
    imgBeamstopDefined.append(false);
    imgBeamstopX.append(beamstopX[current]); imgBeamstopY.append(beamstopY[current]);
    imgBeamstopR.append(beamstopR[current]);
    imgRemark.append("Background");
    imgBackground.append(NULL);
    imgBackgroundLevel.append(1.0);

    updateImagesTable();
  }
}

void Look::removeImage()
{
  sp_image_free(images.at(currentImg));
  img = NULL;
  images.removeAt(currentImg);
  imageNames.removeAt(currentImg);
  imgCenterDefined.removeAt(currentImg);
  imgCenterX.removeAt(currentImg);
  imgCenterY.removeAt(currentImg);
  imgBeamstopDefined.removeAt(currentImg);
  imgBeamstopX.removeAt(currentImg);
  imgBeamstopY.removeAt(currentImg);
  imgBeamstopR.removeAt(currentImg);
  imgBackground.removeAt(currentImg);
  imgBackgroundLevel.removeAt(currentImg);
  for (int i = 0; i < imagesTable->rowCount()-1; i++) {
    if (imgBackground[i] > currentImg) imgBackground[i] -= 1;
    else if (imgBackground[i] == currentImg) imgBackground[i] = NULL;
  }

  /*
  imagesTable->removeCellWidget(currentImg,0);
  imagesTable->removeCellWidget(currentImg,1);
  for (int i = currentImg; i < imagesTable->rowCount()-1; i++) {
    imagesTable->setItem(i,0,imagesTable->takeItem(i+1,0));
    imagesTable->setItem(i,1,imagesTable->takeItem(i+1,0));
  }
  imagesTable->setRangeSelected(QTableWidgetSelectionRange(0,0,imagesTable->rowCount()-1,1),false);
  imagesTable->setRowCount(imagesTable->rowCount()-1);
  */
  updateImagesTable();
}

void Look::setCenter(int on)
{
  updateCenter();
  if (imgFromList) imgCenterDefined[currentImg] = true;
  else centerDefined[current] = true;
  view->setCenter(on);
}

void Look::defineBeamstop(int on)
{
  if (imgFromList) imgBeamstopDefined[currentImg] = true;
  else beamstopDefined[current] = true;
  view->defineBeamstop(on);
  updateBeamstop();
}

void Look::centerToAll()
{
  if (imgFromList) {
    for (int i = 0; i < noOfImages; i++) {
      imgCenterDefined[i] = true;
      imgCenterX[i] = centerX[currentImg];
      imgCenterY[i] = centerY[currentImg];
    }
  } else {
    for (int i = 0; i < noOfImages; i++) {
      centerDefined[i] = true;
      centerX[i] = centerX[current];
      centerY[i] = centerY[current];
    }
  }
}

void Look::beamstopToAll()
{
  if (imgFromList) {
    for (int i = 0; i < noOfImages; i++) {
      imgBeamstopDefined[i] = true;
      imgBeamstopX[i] = imgBeamstopX[currentImg];
      imgBeamstopY[i] = imgBeamstopY[currentImg];
      imgBeamstopR[i] = imgBeamstopR[currentImg];
    }
  } else {
    for (int i = 0; i < noOfImages; i++) {
      beamstopDefined[i] = true;
      beamstopX[i] = beamstopX[current];
      beamstopY[i] = beamstopY[current];
      beamstopR[i] = beamstopR[current];
    }
  }
}

void Look::crop()
{
  if (imgFromList) {
    int x1(0), x2(sp_image_x(img)-1), y1(0), y2(sp_image_y(img)-1);
    
    CropDialog *cropDia = new CropDialog(&x1, &x2, &y1, &y2);
    if (cropDia->exec() == 1) {
      printf("crop %i %i %i %i\n",x1,x2,y1,y2);
      if (temporary != NULL) sp_image_free(temporary);
      temporary = rectangle_crop(img,x1,y1,x2,y2);
      sp_image_free(img);
      img = temporary;
      images[currentImg] = temporary;
      temporary = NULL;
      drawImage();
    }
  }
}

void Look::drawAutocorrelation(int on)
{
  if (on) drawAuto = 1;
  else drawAuto = 0;

  if (showDistanceActive) showDistance(1);
  else showDistance(0);
  drawImage();
}

void Look::setLogScale(int on)
{
  if (on) logScale = 1;
  else logScale = 0;

  drawImage();
}

void Look::setColorscale(int kind)
{
  if (kind == 1) colorscale = COLOR_GRAYSCALE;
  if (kind == 2) colorscale = COLOR_TRADITIONAL;
  if (kind == 3) colorscale = COLOR_HOT;
  if (kind == 4) colorscale = COLOR_RAINBOW;
  if (kind == 5) colorscale = COLOR_JET;
  drawImage();
}

void Look::setMarkedAsBackground(int on)
{
  if (!filesTable->selectedItems().empty()) {
    QTableWidgetItem *newItem;
    int i = filesTable->row(filesTable->selectedItems().first());
    if (on) {
      newItem = new QTableWidgetItem("Background");
    } else {
      newItem = new QTableWidgetItem("");
    }
    newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));    
    filesTable->setItem(i,1,newItem);
  }
}

void Look::setMarkedAsHit(int on)
{
  if (!filesTable->selectedItems().empty()) {
    QTableWidgetItem *newItem;
    int i = filesTable->row(filesTable->selectedItems().first());
    if (on) {
      newItem = new QTableWidgetItem("Hit");
    } else {
      newItem = new QTableWidgetItem("");
    }
    newItem->setFlags(newItem->flags() & (~Qt::ItemIsEditable));
    filesTable->setItem(i,1,newItem);
  }
}

void Look::clearBackgroundFlags()
{
  //QTableWidgetItem *newItem;
  for (int i = 0; i < noOfImages; i++) {
    if (filesTable->item(i,1)->text() == "Background") {
      //newItem = new QTableWidgetItem("");
      filesTable->item(i,1)->setText("");
    }
  }
}

void Look::clearHitFlags()
{
  for (int i = 0; i < noOfImages; i++) {
    if (filesTable->item(i,1)->text() == "Hit") {
      filesTable->item(i,1)->setText("");
    }
  }
}

bool Look::calculateBackground()
{
  Image *tmp1;
  Image *tmp2;
  int i,j;
  int count = 0;
  real sum;
  for (i = 0; i < filesTable->rowCount(); i++)
    if (filesTable->item(i,2)->text() == "Background")
      count++;
  QProgressDialog progress("Calculating background ...", "Abort", 0, count, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setValue(0);
  if (background != NULL) sp_image_free(background);
  //Image * tmp1;
  //Image * tmp2;
  background = sp_image_alloc(sp_image_x(img),sp_image_y(img),1);
  count = 0;
  for (i = 0; i < filesTable->rowCount(); i++) {
    if (filesTable->item(i,1)->text() == "Background") {
      progress.setValue(i);
      if (progress.wasCanceled()) return false;
      count++;
      sum = 0;
      //count++;
      //if (!useSize) tmp1 = sp_image_read(currentDir.absoluteFilePath(filesTable->item(i-1,2)->text()).toAscii(),0);
      //else tmp1 = bilinear_rescale(sp_image_read(currentDir.absoluteFilePath(filesTable->item(i-1,2)->text()).toAscii(),0),sizeX,size,Y);
      //tmp2 = bilinear_rescale(tmp1,sp_image_x(background),sp_image_y(background),1);
      tmp1 = sp_image_read(currentDir.absoluteFilePath(filesTable->item(i-1,3)->text()).toAscii(),0);
      tmp2 = bilinear_rescale(tmp1,sp_image_x(background),sp_image_y(background),1);
      //tmp = bilinear_rescale(sp_image_read(currentDir.absoluteFilePath(filesTable->item(i-1,3)->text()).toAscii(),0), sp_image_x(background), sp_image_y(background),1);
      if (progress.wasCanceled()) {sp_image_free(tmp1); sp_image_free(tmp2); return false;}
      //for (j = 0; j < sp_image_size(background); j++) sum += sp_cabs(tmp2->image->data[i]);
      //sp_image_scale(tmp2,sum / (real) sp_image_size(tmp2));
      //sp_image_add(background,tmp2);
      sp_image_add(background,tmp2);
      if (progress.wasCanceled()) {sp_image_free(tmp1); sp_image_free(tmp2); return false;}
      //sp_image_free(tmp1);
      //sp_image_free(tmp2);
      sp_image_free(tmp1);
      sp_image_free(tmp2);
    }
  }
  sp_image_scale(background,1.0 / (real) count);
  backgroundSum = 0;
  for (j = 0; j < sp_image_size(background); j++) backgroundSum += sp_cabs(background->image->data[i]);
  return true;
}

void Look::showBackground()
{
  if (!backgroundWindow) backgroundWindow = new QMainWindow(this, Qt::SubWindow);
  //backgroundLabel = new QLabel;
  //backgroundLabel->setMinimumWidth(500);
  //backgroundLabel->setMaximumWidth(500);
  backgroundView = new ImageView;
  //backgroundView->setMinimumWidth(500);
  //backgroundView->setMaximumWidth(500);
  if (logScale) colormap_data = sp_image_get_false_color(background,colorscale | LOG_SCALE,-1,(int)(65535.0*range));
  else colormap_data = sp_image_get_false_color(background,colorscale,-1,(int)(65535.0*range));
  backgroundQi = QImage(colormap_data,sp_image_x(img),sp_image_y(img),QImage::Format_RGB32);
  //backgroundQi = backgroundQi.scaled(backgroundLabel->width(),backgroundLabel->height(),Qt::KeepAspectRatio);
  backgroundQi = backgroundQi.scaled(backgroundView->width(),backgroundView->height(),Qt::KeepAspectRatio);
  //backgroundLabel->setPixmap(QPixmap::fromImage(backgroundQi));
  backgroundView->setImage(&backgroundQi);
  //backgroundRangeSlider = new QSlider(Qt::Vertical);
  //backgroundRangeSlider->setRange(0,100);
  //backgroundRangeSlider->setValue(100);
  //connect(backgroundRangeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeBackgroundRange(int)));
  backgroundRange = range;

  //QGridLayout *layout = new QGridLayout;
  //layout->addWidget(backgroundView, 0, 0);
  //layout->addWidget(backgroundRangeSlider, 0, 1);
  //backgroundWindow->setLayout(layout);

  backgroundWindow->setCentralWidget(backgroundView);
  backgroundWindow->show();
  backgroundWindow->activateWindow();
}

void Look::subtractBackground(int on)
{
  if (on) subtract = 1;
  else subtract = 0;
  drawImage();
}

void Look::setBackgroundLevel()
{
  if (!backgroundSlider) {
    backgroundSlider = new BackgroundSlider(1.0, this);
    connect(backgroundSlider, SIGNAL(valueChanged(real)), this, SLOT(changeBackgroundLevel(real)));
  }

  //backgroundSliderWindow->setCentralWidget(backgroundSlider);
  backgroundSlider->show();
  backgroundSlider->activateWindow();
}

void Look::changeBackgroundLevel(real level)
{
  if (!imgFromList)
    backgroundLevel[current] = level;
  else
    imgBackgroundLevel[currentImg] = level;
  drawImage();
}

bool Look::setSize()
{
  bool ok;
  int x = QInputDialog::getInteger(this, tr("New side"), tr("New side of the image (pixels)"), 512, 1, 4096, 1, &ok);
  if (ok) {
    sizeX = x; sizeY = x;
    setUseSize(1);
    return true;
  } 
  return false;
}

void Look::setUseSize(int on)
{
  if (on) useSize = true;
  else useSize = false;

  if (background != NULL && ((sp_image_x(background) != sizeX || sp_image_y(background) != sizeY) || !useSize) &&
      QMessageBox::question(this, tr("New image size"), tr("Recalculate the background for the new size?"),
			    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
    calculateBackground();
  }
  openImageFromTable();
}

void Look::showDistance(int on)
{
  //pass on to view.
  view->showDistance(on, drawAuto, wavelength*sqrt(0.25 + detectorDistance*detectorDistance/detectorX/detectorX));
  if (on) showDistanceActive = true;
  else showDistanceActive = false;

}

void Look::beamstopToMask()
{
  if (imgFromList) {
    for (real x = 0.5 / (real) sp_image_x(images[currentImg]); x < 1.0; x += 1.0 / (real) sp_image_x(images[currentImg])) {
      for (real y = 0.5 / (real) sp_image_y(images[currentImg]); y < 1.0; y += 1.0 / (real) sp_image_y(images[currentImg])) {
	if ((x-imgBeamstopX[currentImg])*(x-imgBeamstopX[currentImg]) +
	    (y-imgBeamstopY[currentImg])*(y-imgBeamstopY[currentImg]) <
	    imgBeamstopR[currentImg]*imgBeamstopR[currentImg]) {
	  sp_i3matrix_set(images[currentImg]->mask,(int) ((x * (real) sp_image_x(images[currentImg])) - 0.0),
			  (int) ((y * (real) sp_image_y(images[currentImg])) - 0.0), 0, 0);
	}
      }
    }
    drawImage();
  }
}

void Look::importMask()
{
  if (imgFromList) {
    QString filename = QFileDialog::getOpenFileName(this);
    if (filename == NULL) return;
    Image *mask = sp_image_read(filename.toAscii(),0);
    if (mask == NULL){
      QMessageBox::warning(this, tr("Import mask"), tr("Error opening the specified file"));
      return;
    }
    if (sp_image_x(mask) != sp_image_x(img) || sp_image_y(mask) != sp_image_y(img)) {
      if (QMessageBox::question(this, tr("Import mask"), tr("The mask does not have the same size as the background. Rescale it?"),
			    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
	if (temporary != NULL) sp_image_free(temporary);
	temporary = bilinear_rescale(mask, sp_image_x(img), sp_image_y(img), 1);
	mask = temporary;
	temporary = NULL;
      }
    }
    for (int i = 0; i < sp_image_size(img); i++) {
      if (sp_real(mask->image->data[i])) img->mask->data[i] = 1;
      else img->mask->data[i] = 0;
    }
  }
}

void Look::showMask(int on)
{
  if (on) showMaskActive = true;
  else showMaskActive = false;
  drawImage();
}

void Look::clearMask()
{
  if (imgFromList) {
    for (int i = 0; i < sp_image_size(img); i++) {
      img->mask->data[i] = 1;
    }
  }
  drawImage();
}

void Look::saturationToMask()
{
  if (imgFromList) {
    real max = 0;
    int i;
    for (i = 0; i < sp_image_size(img); i++) {
      if (sp_cabs(img->image->data[i]) > max) max = sp_cabs(img->image->data[i]);
    }
    for (i = 0; i < sp_image_size(img); i++) {
      if (sp_cabs(img->image->data[i]) >= max) img->mask->data[i] = 0;
    }
  }
  drawImage();
}

void Look::vertLineToMask()
{
  if (imgFromList) {
    view->getVertLine();
  }
}

void Look::vertLineToMaskSlot(real x, real y)
{
  if (imgFromList) {
    int xi = (int) (x*(real)sp_image_x(img));
    if (xi >= 0 && xi < sp_image_x(img)) {
      for (int yi = 0; yi < sp_i3matrix_x(img->mask); yi++) {
	sp_i3matrix_set(img->mask,xi,yi,0,0);
      }
    }
  }
  drawImage();
}

void Look::drawMask(bool on)
{
  view->drawMask(on);
}

void Look::undrawMask(bool on)
{
  view->undrawMask(on);
}

void Look::drawMaskSlot(real x, real y)
{
  if (imgFromList && !drawAuto) {
    int xi = (int) (x*(real)sp_image_x(img));
    int yi = (int) (y*(real)sp_image_y(img));
    for (int xk = xi-pencilSize/2; xk < xi+(pencilSize+1)/2; xk++) {
      for (int yk = yi-pencilSize/2; yk < yi+(pencilSize+1)/2; yk++) {
	if (xk > 0 && xk < sp_image_x(img) && yk > 0 && yk < sp_image_y(img))
	  sp_i3matrix_set(img->mask,xk,yk,0,0);
      }
    }
  }
  drawImage();
}

void Look::undrawMaskSlot(real x, real y)
{
  if (imgFromList && !drawAuto) {
    int xi = (int) (x*(real)sp_image_x(img));
    int yi = (int) (y*(real)sp_image_y(img));
    for (int xk = xi-pencilSize/2; xk < xi+(pencilSize+1)/2; xk++) {
      for (int yk = yi-pencilSize/2; yk < yi+(pencilSize+1)/2; yk++) {
	if (xk > 0 && xk < sp_image_x(img) && yk > 0 && yk < sp_image_y(img))
	  sp_i3matrix_set(img->mask,xk,yk,0,1);
      }
    }
  }
  drawImage();
}

void Look::setPencilSize(int size)
{
  pencilSize = size;
}

void Look::imagesTableChanged(int row, int collumn)
{
  printf("image table changed %i %i\n",row,collumn);
  if (collumn == 0) {
    printf("new name = ");
    printf(imagesTable->item(row,collumn)->text().toAscii());
    printf("\n");
    imageNameChanged(imagesTable->item(row,collumn)->text(), row);
  } else if (collumn == 1) {
    imageBackgroundChanged(atoi(imagesTable->item(row,collumn)->text().toAscii())-1, row);
  } else {
    imageRemarkChanged(imagesTable->item(row,collumn)->text(), row);
  }
}

void Look::imageNameChanged(QString name, int i)
{
  printf("image name changed\n");
  printf(name.toAscii());
  imageNames[i] = name;
  printf("changed name\n");
  //updateImagesTable();
}

void Look::imageBackgroundChanged(int backgroundNumber, int i)
{
  printf("imageBackgroundChanged(%i, %i) called f\n",backgroundNumber, i);
  if (backgroundNumber < 0 || backgroundNumber >= imagesTable->rowCount() || backgroundNumber == i) {
    imagesTable->blockSignals(true);
    imagesTable->item(i,1)->setText("");
    imagesTable->blockSignals(false);
    return;
  }
  imgBackground[i] = backgroundNumber;
}

void Look::imageRemarkChanged(QString text, int i)
{
  imgRemark[i] = text;
}
