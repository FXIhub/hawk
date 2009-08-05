
#ifndef _HAWKGUI_H_
#define _HAWKGUI_H_ 1

#include <QMainWindow>

class QGridLayout;
class QLabel;
class ImageDisplay;
class QActionGroup;
class OptionsTree;

class ProcessDisplay;
class ProcessControl;
class ImageCategory;
class ImageView;
class PlotDisplay;
class QComboBox;

class HawkGUI : public QMainWindow
{
  Q_OBJECT
  public:
  HawkGUI();
  ~HawkGUI();
public slots:
 protected:
  void closeEvent(QCloseEvent *event);
private slots:
  void onFocusedViewChanged(ImageView * view);
  void onRunProcessToggled(bool toggled);
  void onProcessFinished();
  void onDeleteOutputTriggered();
  void onColorBoxChanged(int index);
  void onDisplayBoxChanged(int index);
  void onLockTransformationToggled(bool on);
  void onLockBrowseToggled(bool on);
private:
  void createGUI();
  void createCategories();
  void createControls();
  void connectGUIToControls();

  QWidget * createLeftPanel();
  QWidget * createRightPanel();
  QWidget * processControlButtons();
  void createToolBars();
  void createStatusBar();
  void createActions();

  QGridLayout * topLayout;
  //  QLabel * optionsTree;
  OptionsTree * optionsTree;
  ProcessDisplay * processDisplay;
  ImageDisplay * imageDisplay;
  PlotDisplay * plotDisplay;

  QToolBar * imageDisplayToolBar;
  QToolBar * plotToolBar;
  QToolBar * processToolBar;

  QAction * lockTransformation;
  QAction * lockBrowse;
  QAction * autoUpdateView;
  /*
  QAction * loadImage;
  QAction * shiftImage;
  QAction * fourierTransformImage;
  QAction * fourierTransformSquaredImage;
  QAction * maxContrastImage;
  QAction * logScaleImage;

  
  QAction * displayAmplitudes;
  QAction * displayPhases;
  QAction * displayMask;
  QActionGroup * displayGroup;

  QAction * colorGray;
  QAction * colorJet;
  QAction * colorHot;
  QAction * colorRainbow;
  QAction * colorTraditional;
  QAction * colorWheel;
  QActionGroup * colorGroup;
  */

  QAction * loadLog;

  QAction * runProcess;
  QAction * deleteOutput;

  QComboBox * colorBox;
  QComboBox * displayBox;

  ProcessControl * processControl;
  QList<ImageCategory *>imageCategories;
};

#endif
