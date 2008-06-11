#ifndef LOOK_H
#define LOOK_H

#include <QtGui>
#include <QMainWindow>
#include "imageview.h"
#include "backgroundSlider.h"
#include <spimage.h>


class Look : public QWidget
{
  Q_OBJECT
public:
  Look(QWidget *parent = 0);
  
  public:
    void openDirectory();
  void openImage(QString filename);
  void loadImageComment(QString filename);
  void updateImagesTable();
  void drawImage();

  void exportImage();
  void exportFalseColorImage();
  void exportBackground();
  void exportToDir();
  void setProperties();

  void toImages();
  void backgroundToImages();
  void removeImage();
  void setCenter(int on);
  void defineBeamstop(int on);
  void centerToAll();
  void beamstopToAll();
  void crop();

  void drawAutocorrelation(int on);
  void setLogScale(int on);
  bool setSize();
  void setUseSize(int on);
  void showDistance(int on);

  void setColorscale(int kind);

  void setMarkedAsBackground(int on);
  void setMarkedAsHit(int on);
  void clearBackgroundFlags();
  void clearHitFlags();

  bool calculateBackground();
  void showBackground();
  void subtractBackground(int on);
  void setBackgroundLevel();

  void showMask(int on);  
  void importMask();
  void clearMask();
  void beamstopToMask();
  void saturationToMask();
  void vertLineToMask();
  void drawMask(bool on);
  void undrawMask(bool on);
  void setPencilSize(int size);
  void imageNameChanged(QString name, int i);
  void imageBackgroundChanged(int backgroundNumber, int i);
  void imageRemarkChanged(QString text, int i);

  signals:
  void backgroundChecked(bool);
  void hitChecked(bool);
  void imgFromListChanged(bool);

  public slots:
  void loadAllComments();

  private slots:
  void openImageFromTable();
  void openImageFromList();
  void changeRange(int value);
  void changeBackgroundRange(int value);
  void changeBackgroundLevel(real value);
  void updateCenter();
  void updateBeamstop();
  void vertLineToMaskSlot(real x, real y);
  void drawMaskSlot(real x, real y);
  void undrawMaskSlot(real x, real y);
  void imagesTableChanged(int row, int collumn);

 private:
  Image * img;
  Image *draw;
  Image * background;
  Image * temporary;
  real backgroundSum;
  bool imgFromList;
  bool drawAuto;
  bool showMaskActive;
  bool logScale;
  bool subtract;
  int colorscale;
  unsigned char * colormap_data;
  QMainWindow *backgroundWindow;
  QLabel *backgroundLabel;
  ImageView *view;
  ImageView *backgroundView;
  QImage backgroundQi;
  QImage qi;
  QDir currentDir;
  QTabWidget *leftTab;
  QTableWidget *filesTable;
  QTableWidget *imagesTable;
  //QLabel *imageLabel;
  QTextBrowser *comment;

  QList<Image *> images;
  QList<QString> imageNames;

  int current;
  int currentImg;
  int noOfImages;
  real *backgroundLevel;
  //QMainWindow *backgroundSliderWindow;
  BackgroundSlider *backgroundSlider;

  QSlider *rangeSlider;
  real range;
  QSlider *backgroundRangeSlider;
  real backgroundRange;

  //QInputDialog *sizeDialog;
  bool useSize;
  int sizeX, sizeY;

  real *centerX, *centerY;
  bool *centerDefined;
  real *beamstopX, *beamstopY, *beamstopR;
  bool *beamstopDefined;

  QList<real> imgCenterX, imgCenterY;
  QList<bool> imgCenterDefined;
  QList<real> imgBeamstopX, imgBeamstopY, imgBeamstopR;
  QList<bool> imgBeamstopDefined;
  QList<real> imgBackgroundLevel;
  QList<int> imgBackground;
  QList<QString> imgRemark;

  //detector propertiees
  bool propertiesSet;
  real wavelength;
  real detectorDistance;
  real detectorX, detectorY;

  bool showDistanceActive;
  int pencilSize;
};

#endif
