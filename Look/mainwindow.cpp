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
  autocorrelationAct = new QAction(tr("&Autocorrelation"), this);
  autocorrelationAct->setCheckable(true);
  autocorrelationAct->setShortcut(tr("Ctrl+A"));
  connect(autocorrelationAct, SIGNAL(triggered()), this, SLOT(viewAutocorrelation()));

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

  // Mask
  beamstopToMaskAct = new QAction(tr("&Beamstop to mask"),this);
  connect(beamstopToMaskAct, SIGNAL(triggered()), this, SLOT(maskBeamstopToMask()));
  
  importMaskAct = new QAction(tr("&Import mask"),this);
  connect(importMaskAct, SIGNAL(triggered()), this, SLOT(maskImportMask()));

  showMaskAct = new QAction(tr("&Show mask"),this);
  showMaskAct->setCheckable(true);
  connect(showMaskAct, SIGNAL(triggered()), this, SLOT(maskShowMask()));

  saturationToMaskAct = new QAction(tr("&Saturation to mask"), this);
  connect(saturationToMaskAct, SIGNAL(triggered()), this, SLOT(maskSaturationToMask()));

  vertLineToMaskAct  = new QAction(tr("&Vertical line to mask"), this);
  connect(vertLineToMaskAct, SIGNAL(triggered()), this, SLOT(maskVertLineToMask()));
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
  imageMenu->addAction(removeImageAct);
  imageMenu->addSeparator();
  imageMenu->addAction(setCenterAct);
  imageMenu->addAction(centerToAllAct);
  imageMenu->addAction(defineBeamstopAct);
  imageMenu->addAction(beamstopToAllAct);
  imageMenu->addSeparator();
  imageMenu->addAction(cropAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(autocorrelationAct);
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
  categorizeMenu->addSeparator();
  categorizeMenu->addAction(calculateBackgroundAct);
  categorizeMenu->addAction(showBackgroundAct);
  categorizeMenu->addAction(subtractBackgroundAct);
  categorizeMenu->addAction(setBackgroundLevelAct);

  maskMenu = menuBar()->addMenu(tr("&Mask"));
  maskMenu->addAction(showMaskAct);
  maskMenu->addAction(importMaskAct);
  maskMenu->addSeparator();
  maskMenu->addAction(beamstopToMaskAct);
  maskMenu->addAction(saturationToMaskAct);
  maskMenu->addAction(vertLineToMaskAct);
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
    look->drawAutocorrelation(1);
  } else {
    look->drawAutocorrelation(0);
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

void MainWindow::maskBeamstopToMask()
{
  look->beamstopToMask();
}

void MainWindow::maskImportMask()
{
  look->importMask();
}

void MainWindow::maskShowMask()
{
  if (showMaskAct->isChecked())
    look->showMask(1);
  else
    look->showMask(0);
}

void MainWindow::maskSaturationToMask()
{
  look->saturationToMask();
}

void MainWindow::maskVertLineToMask()
{
  look->vertLineToMask();
}
