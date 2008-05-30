#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "look.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
    public:
  MainWindow (QWidget *parent = 0);

  private slots:
  void imgFromListChanged(bool);

  void fileOpenDirectory();
  void fileGetComments();
  void fileExportToDir();
  void fileExportImage();
  void fileExportFalseColor();
  void fileExportBackground();
  void fileSetProperties();
  void imageToImages();
  void imageRemoveImage();
  void imageSetCenter();
  void imageDefineBeamstop();
  void imageCenterToAll();
  void imageBeamstopToAll();
  void imageCrop();
  void viewAutocorrelation();
  void viewLogScale();
  void viewPredefineSize();
  void viewUsePredefinedSize();
  void viewDistances();
  void colorscaleGrayscale();
  void colorscaleTraditional();
  void colorscaleHot();
  void colorscaleRainbow();
  void colorscaleJet();
  void categorizeBackground();
  void categorizeClearBackground();
  void categorizeClearHit();
  void categorizeHit();
  void categorizeCalculateBackground();
  void categorizeShowBackground();
  void categorizeSubtractBackground();
  void categorizeSetBackgroundLevel();
  void maskBeamstopToMask();
  void maskSaturationToMask();
  void maskVertLineToMask();
  void maskImportMask();
  void maskShowMask();

 private:
  Look *look;

  void createActions();
  void createMenus();

  QMenu *fileMenu;
  QAction *openDirectoryAct;
  QAction *getCommentsAct;
  QAction *exportImageAct;
  QAction *exportFalseColorAct;
  QAction *exportBackgroundAct;
  QAction *exportToDirAct;
  QAction *setPropertiesAct;
  QAction *quitAct;

  QMenu *imageMenu;
  QAction *toImagesAct;
  QAction *removeImageAct;
  QAction *setCenterAct;
  QAction *defineBeamstopAct;
  QAction *centerToAllAct;
  QAction *beamstopToAllAct;
  QAction *cropAct;

  QMenu *viewMenu;
  QAction *autocorrelationAct;
  QAction *logScaleAct;
  QAction *predefineSizeAct;
  QAction *usePredefinedSizeAct;
  QAction *distancesAct;
  
  QMenu *colorMenu;
  QAction *colorscaleGrayscaleAct;
  QAction *colorscaleTraditionalAct;
  QAction *colorscaleHotAct;
  QAction *colorscaleRainbowAct;
  QAction *colorscaleJetAct;

  QMenu *categorizeMenu;
  QAction *backgroundAct;
  QAction *clearBackgroundAct;
  QAction *hitAct;
  QAction *clearHitAct;
  QAction *calculateBackgroundAct;
  QAction *showBackgroundAct;
  QAction *subtractBackgroundAct;
  QAction *setBackgroundLevelAct;

  QMenu *maskMenu;
  QAction *beamstopToMaskAct;
  QAction *saturationToMaskAct;
  QAction *vertLineToMaskAct;
  QAction *importMaskAct;
  QAction *showMaskAct;

};


#endif
