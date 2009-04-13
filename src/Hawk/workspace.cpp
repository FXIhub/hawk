#include "imageItem.h"
#include "workspace.h"
#include "server.h"
#include <spimage.h>
//#include <QGLWidget>
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
  propertiesTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
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

void Workspace::on_propertiesTable_cellActivated(int row, int col){
  bool ok;
  QTableWidgetItem * it =  propertiesTable->item(row,col);
  QString value;
  if(it->data(Qt::UserRole)  == QString("bool_editable")){
    QStringList items;
    items << tr("True") << tr("False");
     value = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
					 propertiesTable->item(row,0)->text(), items, 0, false, &ok);
  }
  if(it->data(Qt::UserRole)  == QString("positive_real_editable")){
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
				       propertiesTable->item(row,0)->text(), it->text().toDouble(), 0, 1000000, 2, &ok);
    value = QString::number(d);
  }
  if(it->data(Qt::UserRole)  == QString("real_editable")){
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
				       propertiesTable->item(row,0)->text(), it->text().toDouble(), -1000000, 1000000, 2, &ok);
    value = QString::number(d);
  }
  if (ok && !value.isEmpty()){ 
    propertiesTable->item(row,col)->setText(value);
  }
}
