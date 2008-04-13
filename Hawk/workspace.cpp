#include "imageItem.h"
#include "workspace.h"
#include "server.h"
#include <spimage.h>
#include <QGLWidget>
#include "mainwindow.h"
#include <QtGui>

Workspace::Workspace(QWidget * parent,MainWindow * main)
  :QWidget(parent)
{
  setupUi(this);
  hboxLayout1->setMargin(0);
  hboxLayout->setMargin(0);
  mainWindow = main;
  setupViewers();
  //  propertiesTable->horizontalHeader()->setStretchLastSection(true);
  QFont f =  propertiesTable->font();
  f.setPointSize(10);
  propertiesTable->setFont(f);
  propertiesTable->verticalHeader()->setVisible(false);
  //  propertiesTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}


void Workspace::setupViewers(){
  preprocessViewer->setup(mainWindow);
}

void Workspace::loadImage(QString filename){
  preprocessViewer->loadImage(filename);
}

QTableWidget * Workspace::getPropertiesTable(){
  return propertiesTable;
}
