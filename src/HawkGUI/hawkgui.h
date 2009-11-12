
#ifndef _HAWKGUI_H_
#define _HAWKGUI_H_ 1

#include <QMainWindow>

class QGridLayout;
class QLabel;
class QComboBox;
class QStackedLayout;
class QActionGroup;
class OptionsTree;

class ImageDisplay;

class ProcessDisplay;
class ProcessControl;
class ImageCategory;
class ImageView;
class PlotDisplay;
class QMenu;

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
  void showPhaserWorkspace();
  void showEditorWorkspace();
  void showStitcherWorkspace();
  void helpAbout();
  void helpAboutQt();
private:
  void createGUI();
  void createCategories();
  void createControls();
  void createMenuBar();
  void connectGUIToControls();
  void loadStyleSheet();
  
  QWidget *createPhaserWorkspace();
  QWidget *createEditorWorkspace();
  QWidget * createLeftPanel();
  QWidget * createRightPanel();
  QWidget * processControlButtons();
  void createToolBars();
  void createStatusBar();
  void createActions();


  QStackedLayout * centralLayout;
  QWidget * phaserWorkspace;
  QWidget * editorWorkspace;
  QWidget * stitcherWorkspace;
  QGridLayout * topLayout;
  //  QLabel * optionsTree;
  OptionsTree * optionsTree;
  ProcessDisplay * processDisplay;
  ImageDisplay * imageDisplay;
  PlotDisplay * plotDisplay;

  QToolBar * workspaceToolBar;
  QToolBar * imageDisplayToolBar;
  QToolBar * plotToolBar;
  QToolBar * processToolBar;

  QAction * lockTransformation;
  QAction * lockBrowse;
  QAction * autoUpdateView;

  QAction * loadLog;

  QAction * runProcess;
  QAction * deleteOutput;

  QActionGroup * workspaceGroup;
  QAction * phaserWorkspaceAction;
  QAction * editorWorkspaceAction;
  QAction * stitcherWorkspaceAction;
  

  QComboBox * colorBox;
  QComboBox * displayBox;

  ProcessControl * processControl;

  QList<ImageCategory *>imageCategories;

  QMenu * m_fileMenu;
  QMenu * m_helpMenu;


};

#endif
