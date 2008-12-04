#ifndef LOOK_H
#define LOOK_H

#include <QtGui>
#include <QMainWindow>
#include "imageview.h"
#include "backgroundSlider.h"
#include <spimage.h>

class Ui_SupportLevel;

typedef enum{OriginalImage=1,AveragedBackground=2,ConstantBackground=4,AdaptativeBackground=8,
	     Autocorrelation=16,SortedAutocorrelation=32,AutocorrelationSupport=64,ResultImage=128}CachedParts;
typedef enum{ViewImage=0,ViewAutocorrelation,ViewBackground,ViewAutocorrelationSupport} ViewType;

class Look : public QWidget
{
  Q_OBJECT
public:
  Look(QMainWindow *parent = 0);
  
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
  void drawView(ViewType viewType);
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

  void setConstantBackground();
  void setAdaptativeBackground();

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

  void setSupportLevel();
  signals:
  void backgroundChecked(bool);
  void hitChecked(bool);
  void imgFromListChanged(bool);
  void directoryOpened(QString);
  void imageChanged(int i);

  public slots:
  void loadAllComments();
  void showImageValueAt(real x, real y);
  void clearImageValue();

  private slots:
  void openImageFromTable();
  void openImageFromList();
  void changeRange();
  void changeBackgroundRange(int value);
  void changeBackgroundLevel(real value);

  void changeConstantBackground(real value);
  void updateCenter();
  void updateBeamstop();
  void vertLineToMaskSlot(real x, real y);
  void drawMaskSlot(real x, real y);
  void undrawMaskSlot(real x, real y);
  void imagesTableChanged(int row, int collumn);
  void recalculateImage(int i);

  void floorSliderChanged();
  void ceilingSliderChanged();
  void blurSliderChanged();

  void floorSpinChanged(double v);
  void ceilingSpinChanged(double v);
  void blurSpinChanged(double v);

  void recalculateButtonPressed();

 private:
  Image * img;
  Image *draw;
  Image * background;
  Image * temporary;
  Image * image_cache;
  Image * original_image_cache;
  Image * autocorrelation_cache;
  Image * background_cache;
  Image * sorted_autocorrelation_cache;
  Image * autocorrelation_support_cache;
  Image * averaged_background_cache;
  Image * total_background_cache;
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

  QMainWindow *mainWindow;

  QList<Image *> images;
  QList<Image *> adaptative_background;
  QList<QString> imageNames;

  int current;
  int currentImg;
  int noOfImages;
  real *backgroundLevel;
  real *constantBackgroundLevel;
  //QMainWindow *backgroundSliderWindow;
  BackgroundSlider *backgroundSlider;

  BackgroundSlider * constantBackgroundSlider;

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
  QList<real> supportCeiling;
  QList<real> supportFloor;
  QList<real> supportBlur;


  QList<real> imgCenterX, imgCenterY;
  QList<bool> imgCenterDefined;
  QList<real> imgBeamstopX, imgBeamstopY, imgBeamstopR;
  QList<bool> imgBeamstopDefined;
  QList<real> imgBackgroundLevel;
  QList<real> imgConstantBackgroundLevel;
  QList<int> imgBackground;
  QList<QString> imgRemark;

  //detector propertiees
  bool propertiesSet;
  real wavelength;
  real detectorDistance;
  real detectorX, detectorY;

  bool showDistanceActive;
  int pencilSize;

  ViewType viewType;

  Ui_SupportLevel * supportLevel;
  QStackedLayout * viewTypeWidgetLayout;
  int cachedImageDirty;
};

#endif
