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
  void imageBackgroundToImages();
  void imageRemoveImage();
  void imageSetCenter();
  void imageDefineBeamstop();
  void imageCenterToAll();
  void imageBeamstopToAll();
  void imageCrop();
  void viewAutocorrelation();
  void viewBackground();
  void viewImage();
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
  void backgroundSetConstantBackground();
  void backgroundSetAdaptativeBackground();
  void maskShowMask();
  void maskImportMask();
  void maskClearMask();
  void maskBeamstopToMask();
  void maskSaturationToMask();
  void maskVertLineToMask();
  void maskDrawMask();
  void maskUndrawMask();
  void maskSetPencilSize1();
  void maskSetPencilSize2();
  void maskSetPencilSize3();
  void maskSetPencilSize4();
  void maskSetPencilSize5();
  void maskSetPencilSize7();
  void maskSetPencilSize10();
  void maskSetPencilSize15();
  void maskSetPencilSize20();
  void maskSetPencilSize30();
  void maskSetPencilSize50();
  void maskSetPencilSize(int size);
  void onDirectoryOpened(QString);
  void viewSupport();
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
  QAction *backgroundToImagesAct;
  QAction *removeImageAct;
  QAction *setCenterAct;
  QAction *defineBeamstopAct;
  QAction *centerToAllAct;
  QAction *beamstopToAllAct;
  QAction *cropAct;

  QMenu *viewMenu;
  QActionGroup * viewTypeGroup;
  QAction *viewImageAct;
  QAction *autocorrelationAct;
  QAction *viewBackgroundAct;
  QAction *viewSupportAct;

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

  QMenu * backgroundMenu;
  QAction *calculateBackgroundAct;
  QAction *showBackgroundAct;
  QAction *subtractBackgroundAct;
  QAction *setBackgroundLevelAct;
  QAction *setConstantBackgroundAct;
  QAction *setAdaptativeBackgroundAct;


  QMenu *maskMenu;
  QAction *showMaskAct;
  QAction *importMaskAct;
  QAction *clearMaskAct;
  QAction *beamstopToMaskAct;
  QAction *saturationToMaskAct;
  QAction *vertLineToMaskAct;
  QAction *drawMaskAct;
  QAction *undrawMaskAct;

  QMenu *pencilSizeMenu;
  QAction *size1Act;
  QAction *size2Act;
  QAction *size3Act;
  QAction *size4Act;
  QAction *size5Act;
  QAction *size7Act;
  QAction *size10Act;
  QAction *size15Act;
  QAction *size20Act;
  QAction *size30Act;
  QAction *size50Act;
  QActionGroup *pencilSizeGroup;

};


#endif
