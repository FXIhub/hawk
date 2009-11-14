#include <QtGui>

#include "hawkgui.h"
#include "geometrycontrol.h"
#include "imagedisplay.h"
#include "optionstree.h"
#include "processdisplay.h"
#include "processcontrol.h"
#include "imagecategory.h"
#include "imageview.h"
#include "plotdisplay.h"
#include "editorworkspace.h"
#include "stitcherworkspace.h"
#include "rpcserver.h"
#include "remotelaunchdialog.h"

HawkGUI::HawkGUI()
  :QMainWindow()
{
  loadStyleSheet();
  createCategories();
  createGUI();
  createControls();
  connectGUIToControls();
  resize(1024,768);
}

HawkGUI::~HawkGUI(){
  int size = imageCategories.size();
  for(int i = 0;i<size;i++){
    delete imageCategories.at(i);
  }
}

void HawkGUI::createGUI(){
  QWidget * centralWidget = new QWidget(this);
  centralLayout = new QStackedLayout(centralWidget);
  centralWidget->setLayout(centralLayout);
  centralLayout->setStackingMode(QStackedLayout::StackOne);
  setCentralWidget(centralWidget);
  phaserWorkspace = createPhaserWorkspace();
  centralLayout->addWidget(phaserWorkspace);
  editorWorkspace = createEditorWorkspace();
  centralLayout->addWidget(editorWorkspace);
  stitcherWorkspace = new StitcherWorkspace(this);
  centralLayout->addWidget(stitcherWorkspace);
  createActions();
  createToolBars();
  createStatusBar();
  createMenuBar();
}

QWidget * HawkGUI::createPhaserWorkspace(){
  QSplitter * splitter = new QSplitter(Qt::Horizontal,this);

  QWidget * leftPanel = createLeftPanel();
  QWidget * rightPanel = createRightPanel();
  
  splitter->addWidget(leftPanel);
  splitter->addWidget(rightPanel);
  splitter->setStretchFactor (0,0);
  splitter->setStretchFactor (1,1);
  return splitter;
}

QWidget * HawkGUI::createEditorWorkspace(){
  return new EditorWorkspace(this);
  
}

QWidget * HawkGUI::createLeftPanel(){
  QWidget * leftPanel = new QWidget(this);
  
  /* Build left panel */
  optionsTree = new OptionsTree(this);
  processDisplay = new ProcessDisplay(this);
  
  QVBoxLayout *vlayout = new QVBoxLayout;
  vlayout->addWidget(optionsTree);
  vlayout->addWidget(processDisplay);
  
  leftPanel->setLayout(vlayout);
  return leftPanel;
}

QWidget * HawkGUI::createRightPanel(){
  imageDisplay = new ImageDisplay(this);
  imageDisplay->setImageCategories(&imageCategories);
  plotDisplay = new PlotDisplay(this);
  
  QSplitter * rightPanel = new QSplitter(Qt::Vertical,centralWidget());
  rightPanel->addWidget(imageDisplay);
  rightPanel->setStretchFactor(0,4);
  rightPanel->addWidget(plotDisplay);
  rightPanel->setStretchFactor(1,1);
  return rightPanel;
}


void HawkGUI::createToolBars(){
  workspaceToolBar = addToolBar(tr("Workspace"));
  workspaceToolBar->setIconSize(QSize(32,32));
  workspaceToolBar->addAction(stitcherWorkspaceAction);
  workspaceToolBar->addAction(editorWorkspaceAction);
  workspaceToolBar->addAction(phaserWorkspaceAction);
  processToolBar = addToolBar(tr("Process"));
  processToolBar->addAction(runProcess);
  processToolBar->addAction(deleteOutput);
  processToolBar->setIconSize(QSize(32,32));
  plotToolBar = addToolBar(tr("Plot"));
  plotToolBar->addAction(loadLog);
  plotToolBar->setIconSize(QSize(32,32));

  imageDisplayToolBar = addToolBar(tr("Image Display"));
  // Make decent sized icons
  QWidget * stretcher = new QWidget(imageDisplayToolBar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  imageDisplayToolBar->addWidget(stretcher);
  imageDisplayToolBar->setIconSize(QSize(32,32));
  imageDisplayToolBar->addAction(lockTransformation);
  imageDisplayToolBar->addAction(lockBrowse);
  imageDisplayToolBar->addAction(autoUpdateView);
  stretcher = new QWidget(imageDisplayToolBar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));

  imageDisplayToolBar->addWidget(stretcher);
}

void HawkGUI::createStatusBar(){
  statusBar()->showMessage(tr("Ready"));
}

void HawkGUI::createMenuBar(){
  QAction *action;
  QSettings settings;
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  action = new QAction("&Quit", this);
  action->setShortcut(tr("CTRL+Q"));
  connect(action, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
  m_fileMenu->addAction(action);


  QActionGroup * actionGroup = new QActionGroup(this);
  actionGroup->setExclusive(true);

  m_settingsMenu = menuBar()->addMenu(tr("&Settings"));
  m_settingsRunMenu = m_settingsMenu->addMenu(tr("&Launch runs"));
  action = new QAction("&Locally", this);
  action->setCheckable(true);
  if(settings.value("ProcessControl/launchMethod").toInt() == ProcessControl::LaunchLocally){
    action->setChecked(true);
  }
  actionGroup->addAction(action);
  connect(action, SIGNAL(triggered()), this, SLOT(settingsRunLocally()));
  m_settingsRunMenu->addAction(action);
  action = new QAction("&Remotely (RPC)", this);
  action->setCheckable(true);
  if(settings.value("ProcessControl/launchMethod").toInt() == ProcessControl::LaunchRemotely){
    action->setChecked(true);
  }
  actionGroup->addAction(action);
  connect(action, SIGNAL(triggered()), this, SLOT(settingsRunRemotely()));
  m_settingsRunMenu->addAction(action);

  action = new QAction("&Remote Launch...", this);
  m_settingsMenu->addAction(action);
  connect(action, SIGNAL(triggered()), this, SLOT(settingsRemoteLaunch()));

  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  action = new QAction("&About...", this);
  connect(action, SIGNAL(triggered()), this, SLOT(helpAbout()));  
  m_helpMenu->addAction(action);
  action = new QAction("&About Qt...", this);
  connect(action, SIGNAL(triggered()), this, SLOT(helpAboutQt()));
  m_helpMenu->addAction(action);

}

void HawkGUI::createActions(){
  lockTransformation = new QAction(QIcon(":/images/lock_zoom.png"), tr("&Lock Transformation"), this);
  lockTransformation->setStatusTip(tr("Translates and zooms all images as one."));

  lockTransformation->setCheckable(true);

  lockTransformation->setChecked(true);
  imageDisplay->setLockedTransformation(true);
  
  connect(lockTransformation,SIGNAL(toggled(bool)),this,SLOT(onLockTransformationToggled(bool)));


  lockBrowse = new QAction(QIcon(":/images/lock_browse.png"), tr("&Lock Browse"), this);
  lockBrowse->setStatusTip(tr("Keep all images in the same iteration."));

  lockBrowse->setCheckable(true);
  connect(lockBrowse,SIGNAL(toggled(bool)),this,SLOT(onLockBrowseToggled(bool)));
  lockBrowse->setChecked(true);  

  autoUpdateView = new QAction(QIcon(":images/image_autoupdate.png"),tr("&Auto Reload"), this);
  autoUpdateView->setStatusTip(tr("Automatically loads the last image created."));
  autoUpdateView->setCheckable(true);
  connect(autoUpdateView,SIGNAL(toggled(bool)),imageDisplay,SLOT(setAutoUpdate(bool)));
  autoUpdateView->setChecked(true);

  loadLog = new QAction(QIcon(":images/log_open.png"),tr("&Load Log"), this);
  loadLog->setStatusTip(tr("Load log file."));
  connect(loadLog,SIGNAL(triggered(bool)),plotDisplay,SLOT(loadUserSelectedLogFile()));

  runProcess = new QAction(QIcon(":/images/exec.png"),tr("&Run"), this);
  runProcess->setCheckable(true);
  runProcess->setStatusTip(tr("Start reconstruction."));
  deleteOutput = new QAction(QIcon(":images/image_delete.png"),tr("&Delete Output"), this);
  deleteOutput->setStatusTip(tr("Delete output images from a directory."));


  workspaceGroup = new QActionGroup(this);
  phaserWorkspaceAction = new QAction(QIcon(":/images/theta.png"),tr("Phaser"),this);
  editorWorkspaceAction = new QAction(QIcon(":/images/package_graphics.png"),tr("Editor"),this);
  stitcherWorkspaceAction = new QAction(QIcon(":/images/stitch.png"),tr("Stitcher"),this);
  workspaceGroup->addAction(phaserWorkspaceAction);
  workspaceGroup->addAction(editorWorkspaceAction);
  workspaceGroup->addAction(stitcherWorkspaceAction);
  editorWorkspaceAction->setCheckable(true);
  phaserWorkspaceAction->setCheckable(true);
  stitcherWorkspaceAction->setCheckable(true);
  phaserWorkspaceAction->setStatusTip(tr("Change to phasing workspace"));
  editorWorkspaceAction->setStatusTip(tr("Change to editor workspace"));
  stitcherWorkspaceAction->setStatusTip(tr("Change to stitcher workspace"));
  connect(editorWorkspaceAction,SIGNAL(triggered()),this,SLOT(showEditorWorkspace()));
  connect(phaserWorkspaceAction,SIGNAL(triggered()),this,SLOT(showPhaserWorkspace()));
  connect(stitcherWorkspaceAction,SIGNAL(triggered()),this,SLOT(showStitcherWorkspace()));
  if(centralLayout->currentWidget() == phaserWorkspace){
    phaserWorkspaceAction->setChecked(true);
  }
  if(centralLayout->currentWidget() == editorWorkspace){
    editorWorkspaceAction->setChecked(true);
  }
  if(centralLayout->currentWidget() == stitcherWorkspace){
    stitcherWorkspaceAction->setChecked(true);
  }
} 


void HawkGUI::createControls(){
  processControl = new ProcessControl(this);
}

void HawkGUI::connectGUIToControls(){
  connect(processDisplay,SIGNAL(runButtonToggled(bool)),runProcess,SLOT(setChecked(bool)));
  connect(runProcess,SIGNAL(toggled(bool)),this,SLOT(onRunProcessToggled(bool)));
  connect(optionsTree,SIGNAL(optionsTreeUpdated(Options *)),processControl,SLOT(setOptions(Options *)));
  connect(processControl,SIGNAL(processFinished()),this,SLOT(onProcessFinished()));
  connect(processControl,SIGNAL(processStarted(QString,QString,ProcessControl *)),imageDisplay,SLOT(onProcessStarted(QString,QString,ProcessControl *)));
  connect(processControl,SIGNAL(processStarted(QString,QString,ProcessControl *)),plotDisplay,SLOT(onProcessStarted(QString,QString,ProcessControl *)));

  connect(imageDisplay,SIGNAL(focusedViewChanged(ImageView *)),this,SLOT(onFocusedViewChanged(ImageView *)));

  connect(deleteOutput,SIGNAL(triggered()),this,SLOT(onDeleteOutputTriggered()));

  // force initial signal
  optionsTree->rebuildTree();
}


void HawkGUI::createCategories(){
  imageCategories.append(new ImageCategory("Calculated Pattern","pattern"));
  imageCategories.append(new ImageCategory("Object","real_out"));
  //  imageCategories.append(new ImageCategory("Phase Image","real_out_phase"));
  imageCategories.append(new ImageCategory("Support","support"));
  imageCategories.append(new ImageCategory("Experimental Pattern","amplitudes"));
  imageCategories.append(new ImageCategory("Autocorrelation","autocorrelation",false));
  imageCategories.append(new ImageCategory("Initial Image","initial_guess",false));
  imageCategories.append(new ImageCategory("Initial Support","initial_support",false));
}



void HawkGUI::onFocusedViewChanged(ImageView * view){
  autoUpdateView->setChecked(view->getAutoUpdate());
}


void HawkGUI::onRunProcessToggled(bool toggle){
  if(toggle){
    runProcess->setIcon(QIcon(":/images/stop.png"));
    processControl->startProcess();
  }else{
    runProcess->setIcon(QIcon(":/images/exec.png"));
    processControl->stopProcess();
  }
  processDisplay->toggleRunButton(toggle);
}

void HawkGUI::onProcessFinished(){
  runProcess->setChecked(false);
  imageDisplay->onProcessStopped();
  plotDisplay->onProcessStopped();
}

void HawkGUI::onDeleteOutputTriggered(){
  QString outputDir(processControl->getOptions()->work_dir);
  QString dir = QFileDialog::getExistingDirectory(this,"Delete output from Directory",outputDir,QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
  if(!dir.isEmpty()){
    processControl->deleteOutputFromDir(dir);
  }  
}

void HawkGUI::onColorBoxChanged(int index){
  int color = colorBox->itemData(index).toInt();
  imageDisplay->setColormap(color);
}


void HawkGUI::onDisplayBoxChanged(int index){
  int display = displayBox->itemData(index).toInt();
  imageDisplay->setDisplay(display);
}

void HawkGUI::onLockTransformationToggled(bool on){
  imageDisplay->setLockedTransformation(on);
}

void HawkGUI::onLockBrowseToggled(bool on){
  imageDisplay->setLockedBrowse(on);
}

void HawkGUI::closeEvent(QCloseEvent *event){
  // Check if there are processes running
  if(processControl){
    if(processControl->isRunning()){
      if(QMessageBox::question(this,"Reconstruction Still Running","There is at least one reconstruction still running. Do you really want to exit?",
			       QMessageBox::Yes|QMessageBox::No,QMessageBox::No) != QMessageBox::Yes){
	event->ignore();
	return;
      }
    }
  }
  event->accept();
}

void HawkGUI::loadStyleSheet(){
#ifdef Q_WS_X11  
  QFile qss(":stylesheet/style_x11.qss");
#endif
#ifdef Q_WS_MAC
  QFile qss(":stylesheet/style_mac.qss");
#endif
#ifdef Q_WS_WIN
  QFile qss(":stylesheet/style.qss");
#endif


  qss.open(QIODevice::ReadOnly);
  ((QApplication *)QApplication::instance())->setStyleSheet(qss.readAll());  
}

void HawkGUI::showPhaserWorkspace(){
  centralLayout->setCurrentWidget(phaserWorkspace);
}

void HawkGUI::showEditorWorkspace(){
  centralLayout->setCurrentWidget(editorWorkspace);
}

void HawkGUI::showStitcherWorkspace(){
  centralLayout->setCurrentWidget(stitcherWorkspace);
}

void HawkGUI::helpAbout(){
  QMessageBox::about(this, "About HawkGUI",
		     "HawkGUI is a graphical interface for the Hawk Image Reconstruction package");
}

void HawkGUI::helpAboutQt(){
  QMessageBox::aboutQt(this);
}

void HawkGUI::settingsRunLocally(){
  QSettings settings;
  settings.setValue("ProcessControl/launchMethod",ProcessControl::LaunchLocally);
  qDebug("Running reconstructions locally");
}

void HawkGUI::settingsRunRemotely(){
  QSettings settings;
  settings.setValue("ProcessControl/launchMethod",ProcessControl::LaunchRemotely);
  qDebug("Running reconstructions remotely");
}

void HawkGUI::settingsRemoteLaunch(){
  qDebug("Remote launch settings opened");
  RemoteLaunchDialog dialog(this);
  dialog.exec();
}
