#include <QtGui>

#include "editor_workspace.h"
#include "imageview.h"

EditorWorkspace::EditorWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  hbox->addWidget(leftSplitter);
  ImageView * editor = new ImageView(this);
  hbox->addWidget(editor);
  QGroupBox * toolBox = new QGroupBox(tr("Tools"),this);
  leftSplitter->addWidget(toolBox);
  propertiesTree = createPropertiesTree();
  leftSplitter->addWidget(propertiesTree);
}


QTreeWidget * EditorWorkspace::createPropertiesTree(){
  /* need a real QTreeView with a model */
  QTreeWidget * tree = new QTreeWidget(this);
  QStringList labels;
  labels << "Property" << "Value";
  tree->setHeaderLabels(labels);
  QTreeWidgetItem * itemPos = new QTreeWidgetItem(QStringList("Position"),0);
  QTreeWidgetItem * itemPosX = new QTreeWidgetItem(itemPos,QStringList("x"),0);
  QTreeWidgetItem * itemPosY = new QTreeWidgetItem(itemPos,QStringList("y"),0);
  itemPosY->setFlags(Qt::ItemIsEditable|Qt::ItemIsEnabled);
  itemPosX->setFlags(Qt::ItemIsEditable|Qt::ItemIsEnabled);
  tree->addTopLevelItem(itemPos);
  //  tree->setAnimated(true);
  tree->setAlternatingRowColors(true);
  tree->setEditTriggers(QAbstractItemView::AllEditTriggers);
  tree->setSelectionMode(QAbstractItemView::SingleSelection);
  //  tree->setFrameStyle(QFrame::Panel);
  return tree;  
}

