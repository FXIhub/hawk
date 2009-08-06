#include <QtGui>

#include "editorworkspace.h"
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


QTreeView * EditorWorkspace::createPropertiesTree(){
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  QStandardItem *parentItem = model->invisibleRootItem();
  for (int i = 0; i < 4; ++i) {
    QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
    QStandardItem *value = new QStandardItem(QString("value %0").arg(i));
    value->setFlags(value->flags() | Qt::ItemIsEditable);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    parentItem->appendRow(QList<QStandardItem *>() << item << value);
    parentItem = item;
  }
  QTreeView *treeView = new QTreeView(this);
  treeView->setModel(model);
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->resizeColumnToContents(0);
  treeView->resizeColumnToContents(1);
  /*
  connect(treeView, SIGNAL(clicked(QModelIndex)),
  this, SLOT(clicked(QModelIndex)));*/

  /*
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
    tree->setAnimated(true);
  tree->setAlternatingRowColors(true);
  tree->setEditTriggers(QAbstractItemView::AllEditTriggers);
  tree->setSelectionMode(QAbstractItemView::SingleSelection);
  //  tree->setFrameStyle(QFrame::Panel);
  */
  return treeView;  
}


void EditorWorkspace::clicked(QModelIndex index){
  if(index.column() == 1){
    qDebug("clicked to edit");
  }
}
