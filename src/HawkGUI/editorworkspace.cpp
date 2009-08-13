#include <QtGui>

#include "editorworkspace.h"
#include "imageeditorview.h"
#include "imageviewpanel.h"
#include "imageitem.h"
#include "editortools.h"

EditorWorkspace::EditorWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  hbox->addWidget(leftSplitter);
  _editorView = new ImageEditorView(this);  
  hbox->addWidget(editorView());
  leftSplitter->addWidget(createTools());
  leftSplitter->addWidget(createPropertiesTree());
  connect(editorView(),SIGNAL(imageItemChanged(ImageItem *)),this,SLOT(loadProperties()));
  editorView()->imageViewPanel()->setVisibility(true);
  editorView()->imageViewPanel()->showSaveButton(true);  
}


QWidget * EditorWorkspace::createTools(){
  return new EditorTools(this);
}

void EditorWorkspace::loadProperties(){
  const QString tag("HawkImage_");
  const QMetaObject *metaobject = editorView()->metaObject();
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(propertiesTree->model());
  model->clear();
  model->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
  int count = metaobject->propertyCount();
  for (int i=0; i<count; ++i) {
     QMetaProperty metaproperty = metaobject->property(i);
     const char *name = metaproperty.name();
     /* 
	This is a prefix to distinguish Hawk Image properties from the
	normal widget properties 
     */
     if(!QString(name).startsWith(tag)){
       continue;
     }
     QVariant var =  editorView()->property(name);
     QStandardItem *parentItem = model->invisibleRootItem();
     Qt::ItemFlags itemValueFlags = Qt::ItemIsSelectable|Qt::ItemIsEnabled;
     if(metaproperty.isWritable()){
       itemValueFlags |= Qt::ItemIsEditable;
     }
     if(var.type() == QVariant::PointF){
       QPointF value = var.toPointF();
       QStandardItem * itemName = new QStandardItem(editorView()->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0 x %1").arg(value.x()).arg(value.y()));
       itemName->setData(value,Qt::UserRole + 1);
       itemName->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags & ~Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       parentItem = itemName;
       itemName = new QStandardItem("x");
       itemValue = new QStandardItem(QString("%0").arg(value.x()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       itemName = new QStandardItem("y");
       itemValue = new QStandardItem(QString("%0").arg(value.y()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Size){
       QSize value = var.toSize();
       QStandardItem * itemName = new QStandardItem(editorView()->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0 x %1").arg(value.width()).arg(value.height()));
       itemName->setData(value,Qt::UserRole + 1);
       itemName->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags & ~Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       parentItem = itemName;
       itemName = new QStandardItem("width");
       itemValue = new QStandardItem(QString("%0").arg(value.width()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       itemName = new QStandardItem("height");
       itemValue = new QStandardItem(QString("%0").arg(value.height()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Bool){
       bool value = var.toBool();
       QStandardItem * itemName = new QStandardItem(editorView()->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem();
       itemValue->setData(value,Qt::UserRole + 1);
       itemValue->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       if(itemValueFlags & Qt::ItemIsEditable){
	 itemValue->setFlags((itemValueFlags | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
       }else{
	 itemValue->setFlags(itemValueFlags);
       }
       if(value){
	 itemValue->setCheckState(Qt::Checked);
       }else{
	 itemValue->setCheckState(Qt::Unchecked);
       }
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Double){
       double value = var.toDouble();
       QStandardItem * itemName = new QStandardItem(editorView()->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0").arg(value));
       itemValue->setData(value,Qt::UserRole + 1);
       itemValue->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }
  }
}

QTreeView * EditorWorkspace::createPropertiesTree(){
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  QTreeView *treeView = new QTreeView(this);
  propertiesTree = treeView;
  treeView->setModel(model);
  loadProperties();
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
  if(((item->flags() & Qt::ItemIsEditable) || (item->flags() & Qt::ItemIsUserCheckable )) == 0){
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
      editorView()->setProperty(property.toAscii().constData(),value);
      combined->setText(QString("%0 x %1").arg(value.x()).arg(value.y()));
      
      qDebug("New value for %s = %f %f",property.toAscii().data(),value.x(),value.y());
    }else{
      qFatal("Can't reach here");
    }
  }else if(var.type() == QVariant::Size){
    /* collect new value from the children */
    if(item->hasChildren()){
      QStandardItem * xItem = item->child(0,1);
      QStandardItem * yItem = item->child(1,1);
      QStandardItem * combined = item->model()->item(item->row(),item->column()+1);
      QSize value = QSize(xItem->text().toInt(),yItem->text().toInt());
      QString property = item->data(Qt::UserRole + 2).toString();
      editorView()->setProperty(property.toAscii().constData(),value);
      combined->setText(QString("%0 x %1").arg(value.width()).arg(value.width()));
      
      qDebug("New value for %s = %d x %d",property.toAscii().data(),value.width(),value.height());
    }else{
      qFatal("Can't reach here");
    }
  }else if(var.type() == QVariant::Double){
    double value = item->text().toDouble();
    QString property = item->data(Qt::UserRole + 2).toString();
    editorView()->setProperty(property.toAscii().constData(),value);
    item->setText(QString("%0").arg(value));		  
    qDebug("New value for %s = %f",property.toAscii().data(),value);
  }else if(var.type() == QVariant::Bool){
    bool value = (item->checkState() == Qt::Checked);
    QString property = item->data(Qt::UserRole + 2).toString();
    editorView()->setProperty(property.toAscii().constData(),value);
    qDebug("New value for %s = %d",property.toAscii().data(),value);
  }
}



ImageEditorView * EditorWorkspace::editorView() const{
  return _editorView;
}
