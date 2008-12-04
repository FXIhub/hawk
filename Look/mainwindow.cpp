#include <QtGui>
//#include <QtCore>
//#include <QTimer>
//#include "mainwindow.moc"
#include "mainwindow.h"
#include "look.h"

MainWindow::MainWindow(QWidget *parent)
{
  look = new Look(this);
  //QMenu menu(this);
  createActions();
  createMenus();
  connect(look, SIGNAL(backgroundChecked(bool)), backgroundAct, SLOT(setChecked(bool)));
  connect(look, SIGNAL(hitChecked(bool)), hitAct, SLOT(setChecked(bool)));

  connect(look, SIGNAL(imgFromListChanged(bool)), this, SLOT(imgFromListChanged(bool)));

  connect(look, SIGNAL(directoryOpened(QString)), this, SLOT(onDirectoryOpened(QString)));

  setCentralWidget(look);
  //look.show();

  
}

void MainWindow::createActions()
{
  // File
  openDirectoryAct = new QAction(tr("&Open directory"), this);
  openDirectoryAct->setShortcut(tr("Ctrl+O"));
  connect(openDirectoryAct, SIGNAL(triggered()), this, SLOT(fileOpenDirectory()));

  getCommentsAct = new QAction(tr("&Get all comments"), this);
  connect(getCommentsAct, SIGNAL(triggered()), this, SLOT(fileGetComments()));
  
  exportImageAct = new QAction(tr("&Export image"), this);
  exportImageAct->setShortcut(tr("Ctrl+E"));
  connect(exportImageAct, SIGNAL(triggered()), this, SLOT(fileExportImage()));

  exportFalseColorAct = new QAction(tr("Export &false color image"), this);
  connect(exportFalseColorAct, SIGNAL(triggered()), this, SLOT(fileExportFalseColor()));

  exportBackgroundAct = new QAction(tr("&Export background"), this);
  exportBackgroundAct->setEnabled(false);
  connect(exportBackgroundAct, SIGNAL(triggered()), this, SLOT(fileExportBackground()));

  exportToDirAct = new QAction(tr("&Export hits to directory"), this);
  connect(exportToDirAct, SIGNAL(triggered()), this, SLOT(fileExportToDir()));

  setPropertiesAct = new QAction(tr("&Set properties"), this);
  connect(setPropertiesAct, SIGNAL(triggered()), this, SLOT(fileSetProperties()));

  quitAct = new QAction(tr("&Quit"), this);
  connect(quitAct, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

  // Image
  toImagesAct = new QAction(tr("&To Images"), this);
  connect(toImagesAct, SIGNAL(triggered()), this, SLOT(imageToImages()));

  backgroundToImagesAct = new QAction(tr("&Background to Images"), this);
  backgroundToImagesAct->setEnabled(false);
  connect(backgroundToImagesAct, SIGNAL(triggered()), this, SLOT(imageBackgroundToImages()));

  removeImageAct = new QAction(tr("&Remove image"), this);
  removeImageAct->setEnabled(false);
  connect(removeImageAct, SIGNAL(triggered()), this, SLOT(imageRemoveImage()));
  
  setCenterAct = new QAction(tr("&Set center"), this);
  setCenterAct->setCheckable(true);
  setCenterAct->setChecked(false);
  connect(setCenterAct, SIGNAL(triggered()), this, SLOT(imageSetCenter()));

  defineBeamstopAct = new QAction(tr("&Define beamstop"), this);
  defineBeamstopAct->setCheckable(true);
  defineBeamstopAct->setChecked(false);
  connect(defineBeamstopAct, SIGNAL(triggered()), this, SLOT(imageDefineBeamstop()));

  centerToAllAct = new QAction(tr("&Use center everywhere"), this);
  connect(centerToAllAct, SIGNAL(triggered()), this, SLOT(imageCenterToAll()));

  beamstopToAllAct = new QAction(tr("&Use beamstop everywhere"), this);
  connect(beamstopToAllAct, SIGNAL(triggered()), this, SLOT(imageBeamstopToAll()));

  cropAct = new QAction(tr("&Crop image"), this);
  connect(cropAct, SIGNAL(triggered()), this, SLOT(imageCrop()));

  // View
  viewImageAct = new QAction(tr("&Image"), this);
  viewImageAct->setCheckable(true);
  viewImageAct->setShortcut(tr("Ctrl+I"));
  connect(viewImageAct, SIGNAL(triggered()), this, SLOT(viewImage()));

  autocorrelationAct = new QAction(tr("&Autocorrelation"), this);
  autocorrelationAct->setCheckable(true);
  autocorrelationAct->setShortcut(tr("Ctrl+A"));
  connect(autocorrelationAct, SIGNAL(triggered()), this, SLOT(viewAutocorrelation()));

  viewBackgroundAct = new QAction(tr("&Background"), this);
  viewBackgroundAct->setCheckable(true);
  viewBackgroundAct->setShortcut(tr("Ctrl+B"));
  connect(viewBackgroundAct, SIGNAL(triggered()), this, SLOT(viewBackground()));

  viewSupportAct = new QAction(tr("Support"), this);
  connect(viewSupportAct, SIGNAL(triggered()), this, SLOT(viewSupport()));
  viewSupportAct->setCheckable(true);


  viewTypeGroup = new QActionGroup(this);
  viewTypeGroup->addAction(viewImageAct);
  viewTypeGroup->addAction(autocorrelationAct);
  viewTypeGroup->addAction(viewBackgroundAct);
  viewTypeGroup->addAction(viewSupportAct);
  viewTypeGroup->setExclusive(true);



  logScaleAct = new QAction(tr("&Log Scale"), this);
  logScaleAct->setShortcut(tr("Ctrl+L"));
  logScaleAct->setCheckable(true);
  connect(logScaleAct, SIGNAL(triggered()), this, SLOT(viewLogScale()));
  
  predefineSizeAct = new QAction(tr("&Define size"),this);
  connect(predefineSizeAct, SIGNAL(triggered()), this, SLOT(viewPredefineSize()));

  usePredefinedSizeAct = new QAction(tr("&Use defined size"),this);
  usePredefinedSizeAct->setCheckable(true);
  connect(usePredefinedSizeAct, SIGNAL(triggered()), this, SLOT(viewUsePredefinedSize()));
  usePredefinedSizeAct->setEnabled(false);

  distancesAct = new QAction(tr("&Show distances"), this);
  distancesAct->setShortcut(tr("Ctrl+D"));
  distancesAct->setCheckable(true);
  distancesAct->setChecked(false);
  distancesAct->setEnabled(false);
  connect(distancesAct, SIGNAL(triggered()), this, SLOT(viewDistances()));

  // Colorscale
  colorscaleGrayscaleAct = new QAction(tr("&Grayscale"), this);
  colorscaleGrayscaleAct->setCheckable(true);
  connect(colorscaleGrayscaleAct, SIGNAL(triggered()), this, SLOT(colorscaleGrayscale()));
  colorscaleGrayscaleAct->setChecked(false);

  colorscaleTraditionalAct = new QAction(tr("&Traditional"), this);
  colorscaleTraditionalAct->setCheckable(true);
  connect(colorscaleTraditionalAct, SIGNAL(triggered()), this, SLOT(colorscaleTraditional()));
  colorscaleTraditionalAct->setChecked(false);

  colorscaleHotAct = new QAction(tr("&Hot"), this);
  colorscaleHotAct->setCheckable(true);
  connect(colorscaleHotAct, SIGNAL(triggered()), this, SLOT(colorscaleHot()));
  colorscaleHotAct->setChecked(false);
  
  colorscaleRainbowAct = new QAction(tr("&Rainbow"), this);
  colorscaleRainbowAct->setCheckable(true);
  connect(colorscaleRainbowAct, SIGNAL(triggered()), this, SLOT(colorscaleRainbow()));
  colorscaleRainbowAct->setChecked(false);
  
  colorscaleJetAct = new QAction(tr("&Jet"), this);
  colorscaleJetAct->setCheckable(true);
  connect(colorscaleJetAct, SIGNAL(triggered()), this, SLOT(colorscaleJet()));
  colorscaleJetAct->setChecked(true);

  //Categorize
  backgroundAct = new QAction(tr("&Background"),this);
  backgroundAct->setShortcut(tr("Ctrl+B"));
  backgroundAct->setCheckable(true);
  connect(backgroundAct, SIGNAL(triggered()), this, SLOT(categorizeBackground()));

  clearBackgroundAct = new QAction(tr("Clear b&ackground flags"), this);
  connect(clearBackgroundAct, SIGNAL(triggered()), this, SLOT(categorizeClearBackground()));
  
  hitAct = new QAction(tr("&Hit"),this);
  hitAct->setShortcut(tr("Ctrl+H"));
  hitAct->setCheckable(true);
  connect(hitAct, SIGNAL(triggered()), this, SLOT(categorizeHit()));

  clearHitAct = new QAction(tr("Clear h&it flags"), this);
  connect(clearHitAct, SIGNAL(triggered()), this, SLOT(categorizeClearHit()));

  calculateBackgroundAct = new QAction(tr("&Calculate background"),this);
  calculateBackgroundAct->setShortcut(tr("Ctrl+C"));
  connect(calculateBackgroundAct, SIGNAL(triggered()), this, SLOT(categorizeCalculateBackground()));

  showBackgroundAct = new QAction(tr("&Show background"),this);
  connect(showBackgroundAct, SIGNAL(triggered()), this, SLOT(categorizeShowBackground()));
  showBackgroundAct->setEnabled(false);

  subtractBackgroundAct = new QAction(tr("&Subtract background"),this);
  subtractBackgroundAct->setCheckable(true);
  subtractBackgroundAct->setShortcut(tr("Ctrl+S"));
  connect(subtractBackgroundAct, SIGNAL(triggered()), this, SLOT(categorizeSubtractBackground()));
  subtractBackgroundAct->setEnabled(false);

  setBackgroundLevelAct = new QAction(tr("Set background level"), this);
  connect(setBackgroundLevelAct, SIGNAL(triggered()), this, SLOT(categorizeSetBackgroundLevel()));
  setBackgroundLevelAct->setEnabled(false);

  setConstantBackgroundAct = new QAction(tr("Set constant background"), this);
  connect(setConstantBackgroundAct, SIGNAL(triggered()), this, SLOT(backgroundSetConstantBackground()));
  setConstantBackgroundAct->setEnabled(false);

  setAdaptativeBackgroundAct = new QAction(tr("Set adaptative background"), this);
  connect(setAdaptativeBackgroundAct, SIGNAL(triggered()), this, SLOT(backgroundSetAdaptativeBackground()));
  setAdaptativeBackgroundAct->setEnabled(false);

  // Mask
  showMaskAct = new QAction(tr("&Show mask"),this);
  showMaskAct->setCheckable(true);
  showMaskAct->setShortcut(tr("Ctrl+M"));
  connect(showMaskAct, SIGNAL(triggered()), this, SLOT(maskShowMask()));

  importMaskAct = new QAction(tr("&Import mask"),this);
  connect(importMaskAct, SIGNAL(triggered()), this, SLOT(maskImportMask()));
  
  clearMaskAct = new QAction(tr("&Clear mask"), this);
  connect(clearMaskAct, SIGNAL(triggered()), this, SLOT(maskClearMask()));

  beamstopToMaskAct = new QAction(tr("&Beamstop to mask"),this);
  connect(beamstopToMaskAct, SIGNAL(triggered()), this, SLOT(maskBeamstopToMask()));

  saturationToMaskAct = new QAction(tr("&Saturation to mask"), this);
  connect(saturationToMaskAct, SIGNAL(triggered()), this, SLOT(maskSaturationToMask()));

  vertLineToMaskAct  = new QAction(tr("&Vertical line to mask"), this);
  vertLineToMaskAct->setShortcut(tr("Ctrl+V"));
  connect(vertLineToMaskAct, SIGNAL(triggered()), this, SLOT(maskVertLineToMask()));

  drawMaskAct = new QAction(tr("&Draw mask"), this);
  drawMaskAct->setCheckable(true);
  connect(drawMaskAct, SIGNAL(triggered()), this, SLOT(maskDrawMask()));

  undrawMaskAct = new QAction(tr("&Undraw mask"), this);
  undrawMaskAct->setCheckable(true);
  connect(undrawMaskAct, SIGNAL(triggered()), this, SLOT(maskUndrawMask()));


  size1Act = new QAction(tr("1"),this);
  size2Act = new QAction(tr("2"),this);
  size3Act = new QAction(tr("3"),this);
  size4Act = new QAction(tr("4"),this);
  size5Act = new QAction(tr("5"),this);
  size7Act = new QAction(tr("7"),this);
  size10Act = new QAction(tr("10"),this);
  size15Act = new QAction(tr("15"),this);
  size20Act = new QAction(tr("20"),this);
  size30Act = new QAction(tr("30"),this);
  size50Act = new QAction(tr("50"),this);

  size1Act->setCheckable(true);
  size2Act->setCheckable(true);
  size3Act->setCheckable(true);
  size4Act->setCheckable(true);
  size5Act->setCheckable(true);
  size7Act->setCheckable(true);
  size10Act->setCheckable(true);
  size15Act->setCheckable(true);
  size20Act->setCheckable(true);
  size30Act->setCheckable(true);
  size50Act->setCheckable(true);

  pencilSizeGroup = new QActionGroup(this);
  pencilSizeGroup->addAction(size1Act);
  pencilSizeGroup->addAction(size2Act);
  pencilSizeGroup->addAction(size3Act);
  pencilSizeGroup->addAction(size4Act);
  pencilSizeGroup->addAction(size5Act);
  pencilSizeGroup->addAction(size7Act);
  pencilSizeGroup->addAction(size10Act);
  pencilSizeGroup->addAction(size15Act);
  pencilSizeGroup->addAction(size20Act);
  pencilSizeGroup->addAction(size30Act);
  pencilSizeGroup->addAction(size50Act);
  size3Act->setChecked(true);

  connect(size1Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize1()));
  connect(size2Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize2()));
  connect(size3Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize3()));
  connect(size4Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize4()));
  connect(size5Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize5()));
  connect(size7Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize7()));
  connect(size10Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize10()));
  connect(size15Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize15()));
  connect(size20Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize20()));
  connect(size30Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize30()));
  connect(size50Act, SIGNAL(triggered()), this, SLOT(maskSetPencilSize50()));

}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openDirectoryAct);
  fileMenu->addAction(getCommentsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exportImageAct);
  fileMenu->addAction(exportFalseColorAct);
  fileMenu->addAction(exportBackgroundAct);
  fileMenu->addAction(exportToDirAct);
  fileMenu->addSeparator();
  fileMenu->addAction(setPropertiesAct);
  fileMenu->addAction(quitAct);

  imageMenu = menuBar()->addMenu(tr("&Image"));
  imageMenu->addAction(toImagesAct);
  imageMenu->addAction(backgroundToImagesAct);
  imageMenu->addAction(removeImageAct);
  imageMenu->addSeparator();
  imageMenu->addAction(setCenterAct);
  imageMenu->addAction(centerToAllAct);
  imageMenu->addAction(defineBeamstopAct);
  imageMenu->addAction(beamstopToAllAct);
  imageMenu->addSeparator();
  imageMenu->addAction(cropAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(viewImageAct);
  viewMenu->addAction(autocorrelationAct);
  viewMenu->addAction(viewBackgroundAct);
  viewMenu->addAction(viewSupportAct);
  viewMenu->addSeparator();
  viewMenu->addAction(logScaleAct);
  viewMenu->addSeparator();
  viewMenu->addAction(predefineSizeAct);
  viewMenu->addAction(usePredefinedSizeAct);
  viewMenu->addAction(distancesAct);
  
  colorMenu = menuBar()->addMenu(tr("&Color"));
  colorMenu->addAction(colorscaleGrayscaleAct);
  colorMenu->addAction(colorscaleTraditionalAct);
  colorMenu->addAction(colorscaleHotAct);
  colorMenu->addAction(colorscaleRainbowAct);
  colorMenu->addAction(colorscaleJetAct);

  categorizeMenu = menuBar()->addMenu(tr("&Categorize"));
  categorizeMenu->addAction(backgroundAct);
  categorizeMenu->addAction(clearBackgroundAct);
  categorizeMenu->addAction(hitAct);
  categorizeMenu->addAction(clearHitAct);

  maskMenu = menuBar()->addMenu(tr("&Mask"));
  maskMenu->addAction(showMaskAct);
  maskMenu->addAction(importMaskAct);
  maskMenu->addAction(clearMaskAct);
  maskMenu->addSeparator();
  maskMenu->addAction(beamstopToMaskAct);
  maskMenu->addAction(saturationToMaskAct);
  maskMenu->addAction(vertLineToMaskAct);
  maskMenu->addAction(drawMaskAct);
  maskMenu->addAction(undrawMaskAct);

  backgroundMenu = menuBar()->addMenu(tr("&Background"));
  backgroundMenu->addAction(calculateBackgroundAct);
  backgroundMenu->addAction(showBackgroundAct);
  backgroundMenu->addAction(subtractBackgroundAct);
  backgroundMenu->addAction(setBackgroundLevelAct);
  backgroundMenu->addAction(setConstantBackgroundAct);
  backgroundMenu->addAction(setAdaptativeBackgroundAct);
  
  pencilSizeMenu = maskMenu->addMenu(tr("&Pencil size"));
  pencilSizeMenu->addAction(size1Act);
  pencilSizeMenu->addAction(size2Act);
  pencilSizeMenu->addAction(size3Act);
  pencilSizeMenu->addAction(size4Act);
  pencilSizeMenu->addAction(size5Act);
  pencilSizeMenu->addAction(size7Act);
  pencilSizeMenu->addAction(size10Act);
  pencilSizeMenu->addAction(size15Act);
  pencilSizeMenu->addAction(size20Act);
  pencilSizeMenu->addAction(size30Act);
  pencilSizeMenu->addAction(size50Act);

}

void MainWindow::imgFromListChanged(bool value)
{
  toImagesAct->setEnabled(!value);
  removeImageAct->setEnabled(value);
}

void MainWindow::fileOpenDirectory()
{
  look->openDirectory();
}

void MainWindow::fileGetComments()
{
  QTimer::singleShot(0, look, SLOT(loadAllComments()));
}

void MainWindow::fileExportImage()
{
  look->exportImage();
}

void MainWindow::fileExportFalseColor()
{
  look->exportFalseColorImage();
}

void MainWindow::fileExportBackground()
{
  look->exportBackground();
}
   

void MainWindow::fileExportToDir()
{
  look->exportToDir();
}

void MainWindow::fileSetProperties()
{
  look->setProperties();
  distancesAct->setEnabled(true);
}

void MainWindow::imageToImages()
{
  look->toImages();
}

void MainWindow::imageBackgroundToImages()
{
  look->backgroundToImages();
}

void MainWindow::imageRemoveImage()
{
  look->removeImage();
}

void MainWindow::imageSetCenter()
{
  if (setCenterAct->isChecked()) {
    look->setCenter(1);
  } else {
    look->setCenter(0);
  }
}

void MainWindow::imageDefineBeamstop()
{
  if (defineBeamstopAct->isChecked()) {
    look->defineBeamstop(1);
  } else {
    look->defineBeamstop(0);
  }
}

void MainWindow::imageCenterToAll()
{
  look->centerToAll();
}

void MainWindow::imageBeamstopToAll()
{
  look->beamstopToAll();
}

void MainWindow::imageCrop()
{
  look->crop();
}

void MainWindow::viewAutocorrelation()
{
  if (autocorrelationAct->isChecked()) {
    look->drawView(ViewAutocorrelation);
  }
}

void MainWindow::viewBackground(){
  if (viewBackgroundAct->isChecked()){
    look->drawView(ViewBackground);
  }
}
void MainWindow::viewImage(){
  if (viewImageAct->isChecked()){
    look->drawView(ViewImage);
  }
}

void MainWindow::viewLogScale()
{
  if (logScaleAct->isChecked()) {
    look->setLogScale(1);
  } else {
    look->setLogScale(0);
  }
}

void MainWindow::viewPredefineSize()
{
  if (look->setSize()) {
    usePredefinedSizeAct->setEnabled(true);
    usePredefinedSizeAct->setChecked(true);
  }
}

void MainWindow::viewUsePredefinedSize()
{
  if (usePredefinedSizeAct->isChecked()) {
    look->setUseSize(true);
  } else {
    look->setUseSize(false);
  }
}

void MainWindow::viewDistances()
{
  if (distancesAct->isChecked()) {
    look->showDistance(1);
  } else {
    look->showDistance(0);
  }
}

void MainWindow::colorscaleGrayscale()
{
  look->setColorscale(1);
  colorscaleGrayscaleAct->setChecked(true);
  colorscaleTraditionalAct->setChecked(false);
  colorscaleHotAct->setChecked(false);
  colorscaleRainbowAct->setChecked(false);
  colorscaleJetAct->setChecked(false);
}

void MainWindow::colorscaleTraditional()
{
  look->setColorscale(2);
  colorscaleGrayscaleAct->setChecked(false);
  colorscaleTraditionalAct->setChecked(true);
  colorscaleHotAct->setChecked(false);
  colorscaleRainbowAct->setChecked(false);
  colorscaleJetAct->setChecked(false);
}

void MainWindow::colorscaleHot()
{
  look->setColorscale(3);
  colorscaleGrayscaleAct->setChecked(false);
  colorscaleTraditionalAct->setChecked(false);
  colorscaleHotAct->setChecked(true);
  colorscaleRainbowAct->setChecked(false);
  colorscaleJetAct->setChecked(false);
}

void MainWindow::colorscaleRainbow()
{
  look->setColorscale(4);
  colorscaleGrayscaleAct->setChecked(false);
  colorscaleTraditionalAct->setChecked(false);
  colorscaleHotAct->setChecked(false);
  colorscaleRainbowAct->setChecked(true);
  colorscaleJetAct->setChecked(false);
}

void MainWindow::colorscaleJet()
{
  look->setColorscale(5);
  colorscaleGrayscaleAct->setChecked(false);
  colorscaleTraditionalAct->setChecked(false);
  colorscaleHotAct->setChecked(false);
  colorscaleRainbowAct->setChecked(false);
  colorscaleJetAct->setChecked(true);
}

  
void MainWindow::categorizeBackground()
{
  if (backgroundAct->isChecked()) {
    look->setMarkedAsBackground(1);
    hitAct->setChecked(0);
  } else {
    look->setMarkedAsBackground(0);
  }
}

void MainWindow::categorizeClearBackground()
{
  look->clearBackgroundFlags();
  if (backgroundAct->isChecked()) backgroundAct->setChecked(false);
}

void MainWindow::categorizeHit()
{
  if (hitAct->isChecked()) {
    look->setMarkedAsHit(1);
    backgroundAct->setChecked(0);
  } else {
    look->setMarkedAsHit(0);
  }
}

void MainWindow::categorizeClearHit()
{
  look->clearHitFlags();
  if (hitAct->isChecked()) hitAct->setChecked(false);
}

void MainWindow::categorizeCalculateBackground()
{
  if(look->calculateBackground()) {
    showBackgroundAct->setEnabled(true);
    subtractBackgroundAct->setEnabled(true);
    setBackgroundLevelAct->setEnabled(true);
    exportBackgroundAct->setEnabled(true);
    backgroundToImagesAct->setEnabled(true);
  }
}

void MainWindow::categorizeShowBackground()
{
  look->showBackground();
}

void MainWindow::categorizeSubtractBackground()
{
  if (subtractBackgroundAct->isChecked())
    look->subtractBackground(1);
  else
    look->subtractBackground(0);
}

void MainWindow::categorizeSetBackgroundLevel()
{
  look->setBackgroundLevel();
}


void MainWindow::backgroundSetConstantBackground()
{
  look->setConstantBackground();
}

void MainWindow::backgroundSetAdaptativeBackground()
{
  look->setAdaptativeBackground();
}

void MainWindow::maskShowMask()
{
  if (showMaskAct->isChecked())
    look->showMask(1);
  else
    look->showMask(0);
}

void MainWindow::maskImportMask()
{
  look->importMask();
}

void MainWindow::maskClearMask()
{
  look->clearMask();
}

void MainWindow::maskBeamstopToMask()
{
  look->beamstopToMask();
}

void MainWindow::maskSaturationToMask()
{
  look->saturationToMask();
}

void MainWindow::maskVertLineToMask()
{
  look->vertLineToMask();
}

void MainWindow::maskDrawMask()
{
  undrawMaskAct->setChecked(false);
  look->drawMask(drawMaskAct->isChecked());
}

void MainWindow::maskUndrawMask()
{
  drawMaskAct->setChecked(false);
  look->undrawMask(undrawMaskAct->isChecked());
}

void MainWindow::maskSetPencilSize1() {maskSetPencilSize(1);}
void MainWindow::maskSetPencilSize2() {maskSetPencilSize(2);}
void MainWindow::maskSetPencilSize3() {maskSetPencilSize(3);}
void MainWindow::maskSetPencilSize4() {maskSetPencilSize(4);}
void MainWindow::maskSetPencilSize5() {maskSetPencilSize(5);}
void MainWindow::maskSetPencilSize7() {maskSetPencilSize(7);}
void MainWindow::maskSetPencilSize10() {maskSetPencilSize(10);}
void MainWindow::maskSetPencilSize15() {maskSetPencilSize(15);}
void MainWindow::maskSetPencilSize20() {maskSetPencilSize(20);}
void MainWindow::maskSetPencilSize30() {maskSetPencilSize(30);}
void MainWindow::maskSetPencilSize50() {maskSetPencilSize(50);}

void MainWindow::maskSetPencilSize(int size)
{
  look->setPencilSize(size);
}


void MainWindow::onDirectoryOpened(QString dir){
  setConstantBackgroundAct->setEnabled(true);
  setAdaptativeBackgroundAct->setEnabled(true);
}

void MainWindow::viewSupport(){
  if (viewSupportAct->isChecked()) {
    look->drawView(ViewAutocorrelationSupport);
  }
}
