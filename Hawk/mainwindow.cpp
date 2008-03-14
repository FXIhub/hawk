#include "mainwindow.h"
#include "server.h"
#include "ui_Hawk.h"
#include "workspace.h"
#include <QFileDialog>
#include <spimage.h>

MainWindow::MainWindow(QMainWindow * parent)
  :QMainWindow(parent)
{
  setupUi(this);
  QActionGroup *myActionGroup = new QActionGroup(this);
  myActionGroup->addAction(actionPreprocess);
  myActionGroup->addAction(actionReconstruct);
  myActionGroup->addAction(actionAnalyse);
  myActionGroup->setExclusive(true);
  workspaces.append(new Workspace(tab));
  vboxLayout1->addWidget(workspaces.last()); 
  vboxLayout1->setMargin(4); 
  server = new Server(this,1050);
}

void MainWindow::on_actionOpen_triggered(bool checked){
  QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
			       "",
			       tr("Images (*.h5 *.tiff *.tif)"));
  if(filename.isEmpty()){
    return;
  }
  Workspace * ws = workspaces[workspaceTab->currentIndex()];
  ws->loadImage(filename);
}
