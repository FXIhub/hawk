#include <QtGui>

#include "editorworkspace.h"
#include "imageeditorview.h"

EditorWorkspace::EditorWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  hbox->addWidget(leftSplitter);
  editorView = new ImageEditorView(this);
  hbox->addWidget(editorView);
  QGroupBox * toolBox = new QGroupBox(tr("Tools"),this);
  leftSplitter->addWidget(toolBox);
  propertiesTree = createPropertiesTree();
  leftSplitter->addWidget(propertiesTree);
}


QTreeView * EditorWorkspace::createPropertiesTree(){
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  model->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
  const QMetaObject *metaobject = editorView->metaObject();
  int count = metaobject->propertyCount();
  for (int i=0; i<count; ++i) {
     QMetaProperty metaproperty = metaobject->property(i);
     const char *name = metaproperty.name();
     QVariant var =  editorView->property(name);
     QStandardItem *parentItem = model->invisibleRootItem();
     if(var.type() == QVariant::PointF){
       QPointF value = var.toPointF();
       QStandardItem * itemName = new QStandardItem(editorView->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0 x %1").arg(value.x()).arg(value.y()));
       itemName->setData(value,Qt::UserRole + 1);
       itemName->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValue->flags() & ~Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       parentItem = itemName;
       itemName = new QStandardItem("x");
       itemValue = new QStandardItem(QString("%0").arg(value.x()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValue->flags() | Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       itemName = new QStandardItem("y");
       itemValue = new QStandardItem(QString("%0").arg(value.y()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValue->flags() | Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }
  }
  QTreeView *treeView = new QTreeView(this);
  treeView->setModel(model);
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->resizeColumnToContents(0);
  treeView->resizeColumnToContents(1);
  connect(model,SIGNAL(itemChanged(QStandardItem * )),this,SLOT(onItemChanged(QStandardItem *)));
  return treeView;  
}


void EditorWorkspace::clicked(QModelIndex index){
  if(index.column() == 1){
    qDebug("clicked to edit");
  }
}


void EditorWorkspace::onItemChanged(QStandardItem * item){
  if((item->flags() & Qt::ItemIsEditable) == 0){
    return;
  }
  qDebug("item changed");
  if(!item->data().isValid()){
    item = item->parent();
  }
  if(!item->data().isValid()){
    qFatal("Can't reach here");
  }
  QVariant var = item->data();
  if(var.type() == QVariant::PointF){
    /* collect new value from the children */
    if(item->hasChildren()){
      QStandardItem * xItem = item->child(0,1);
      QStandardItem * yItem = item->child(1,1);
      QStandardItem * combined = item->model()->item(item->row(),item->column()+1);
      QPointF value = QPointF(xItem->text().toDouble(),yItem->text().toDouble());
      QString property = item->data(Qt::UserRole + 2).toString();
      editorView->setProperty(property.toAscii().constData(),value);
      combined->setText(QString("%0 x %1").arg(value.x()).arg(value.y()));
      
      qDebug("New value for %s = %f %f",property.toAscii().data(),value.x(),value.y());
    }else{
      qFatal("Can't reach here");
    }
  }

}
