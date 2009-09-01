#include <QtGui>
#include "stitcherworkspace.h"
#include "stitcherview.h"
#include "imageitem.h"


Q_DECLARE_METATYPE(ImageItem *);

StitcherWorkspace::StitcherWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  _stitcherView = new StitcherView(this);    
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  hbox->addWidget(leftSplitter);
  leftSplitter->addWidget(createToolBar());
  leftSplitter->addWidget(createGeometryTree());
  connect(_stitcherView,SIGNAL(imageItemGeometryChanged(ImageItem *)),this,SLOT(loadGeometry()));
  //  QHBoxLayout * hbox = new QHBoxLayout(this);
  hbox->addWidget(_stitcherView); 
}

QWidget * StitcherWorkspace::createToolBar(){
  QGroupBox * ret = new QGroupBox(this);
  ret->setTitle(tr("Tools"));
  QGridLayout * layout = new QGridLayout(ret);
  ret->setLayout(layout);
  QSize iconSize = QSize(22,22);

  QToolButton * arrow = new QToolButton(this);
  arrow->setIcon(QIcon(":images/cursor_arrow.png"));
  arrow->setToolTip(tr("Drag images around"));
  arrow->setIconSize(iconSize);
  connect(arrow,SIGNAL(clicked(bool)),this,SLOT(onArrowClicked()));
  layout->addWidget(arrow,0,0);

  QToolButton * stitch = new QToolButton(this);
  stitch->setIcon(QIcon(":images/stitch.png"));
  stitch->setToolTip(tr("Combine images in a single one mantaining relative position"));
  stitch->setIconSize(iconSize);
  connect(stitch,SIGNAL(clicked(bool)),this,SLOT(onStitchClicked()));
  layout->addWidget(stitch,0,5);

  QToolButton * line = new QToolButton(this);
  line->setIcon(QIcon(":images/add_line.png"));
  line->setToolTip(tr("Draw guide line"));
  line->setIconSize(iconSize);
  connect(line,SIGNAL(clicked(bool)),this,SLOT(onLineClicked()));
  layout->addWidget(line,0,1);

  QToolButton * circle = new QToolButton(this);
  circle->setIcon(QIcon(":images/add_circle.png"));
  circle->setToolTip(tr("Draw guide circle"));
  circle->setIconSize(iconSize);
  connect(circle,SIGNAL(clicked(bool)),this,SLOT(onCircleClicked()));
  layout->addWidget(circle,0,2);

  QToolButton * rotate = new QToolButton(this);
  rotate->setIcon(QIcon(":images/undo.png"));
  rotate->setToolTip(tr("Rotate image 180 degrees"));
  rotate->setIconSize(iconSize);
  connect(rotate,SIGNAL(clicked(bool)),this,SLOT(onRotateClicked()));
  layout->addWidget(rotate,0,3);

  QToolButton * clear = new QToolButton(this);
  clear->setIcon(QIcon(":images/clear.png"));
  clear->setToolTip(tr("Remove helper lines"));
  clear->setIconSize(iconSize);
  connect(clear,SIGNAL(clicked(bool)),_stitcherView,SLOT(clearHelpers()));
  layout->addWidget(clear,0,4);

  QToolButton * clearAll = new QToolButton(this);
  clearAll->setIcon(QIcon(":images/edit-delete.png"));
  clearAll->setToolTip(tr("Clear workspace"));
  clearAll->setIconSize(iconSize);
  connect(clearAll,SIGNAL(clicked(bool)),_stitcherView,SLOT(clearAll()));
  layout->addWidget(clearAll,0,5);


  QToolButton * addPoint = new QToolButton(this);
  addPoint->setIcon(QIcon(":images/add_point.png"));
  addPoint->setToolTip(tr("Add control point to image"));
  addPoint->setIconSize(iconSize);
  connect(addPoint,SIGNAL(clicked(bool)),this,SLOT(onAddControlPointClicked()));
  layout->addWidget(addPoint,1,0);

  QToolButton * deletePoint = new QToolButton(this);
  deletePoint->setIcon(QIcon(":images/delete_point.png"));
  deletePoint->setToolTip(tr("Delete control point from image"));
  deletePoint->setIconSize(iconSize);
  connect(deletePoint,SIGNAL(clicked(bool)),this,SLOT(onDeleteControlPointClicked()));
  layout->addWidget(deletePoint,1,1);
 
 
  layout->setColumnStretch(11,100);
  layout->setRowStretch(10,100);
  return ret;
}

void StitcherWorkspace::onStitchClicked(){
  QList<QGraphicsItem *> it = _stitcherView->items();
  if(it.size() < 2){
    return;
  }
  QRectF combined;
  for(int i = 0; i < it.size(); i++){
    if(QString("ImageItem") == it[i]->data(0)){
      /* we have an image item */
      ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it[i]);
      combined = combined.united(ii->sceneBoundingRect());
    }
  }
  /* we're gonna assume they all have the same scaling */
  combined =  _stitcherView->selectedImage()->mapRectFromScene(combined);
  qDebug("Combined with relative positions:");
  Image * a = sp_image_alloc(combined.width(),combined.height(),1);
  for(int i = 0; i < it.size();i++){
    if(QString("ImageItem") == it[i]->data(0)){
      QPointF p =  it[i]->mapToScene(0,0);
      QPointF local_p = _stitcherView->selectedImage()->mapFromScene(p);
      qDebug("x = %f y = %f",local_p.x()-combined.x(),local_p.y()-combined.y());
    }
  }
  for(int x = 0;x<sp_image_x(a);x++){
    for(int y = 0;y<sp_image_y(a);y++){
      int mask = 0;
      Complex value = sp_cinit(0,0);
      QPointF p =  _stitcherView->selectedImage()->mapToScene(QPointF(combined.x()+x,combined.y()+y));
      for(int i = 0; i < it.size();i++){
	if(QString("ImageItem") == it[i]->data(0)){
	  /* we have an image item */
	  ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it[i]);
	  
	  QPointF local_p = it[i]->mapFromScene(p);
	  if(it[i]->contains(local_p)){	  
	    /* we're in business */
	    value = sp_cadd(value,sp_image_get(ii->getImage(),local_p.x(),local_p.y(),0));
	    mask++;
	  }	
	}
      }
      if(mask){
	sp_cscale(value,1.0/mask);
      }
      sp_image_set(a,x,y,0,value);
      sp_image_mask_set(a,x,y,0,mask);
    }
  }
  ImageItem * item = new ImageItem(a,QString(),_stitcherView,NULL);
  _stitcherView->addImage(item);    
  item->update();  
}


void StitcherWorkspace::onAddControlPointClicked(){
  _stitcherView->setMode(StitcherView::AddPoint);
}

void StitcherWorkspace::onDeleteControlPointClicked(){
  _stitcherView->setMode(StitcherView::DeletePoint);
}


void StitcherWorkspace::onCircleClicked(){
  _stitcherView->setMode(StitcherView::Circle);
}

void StitcherWorkspace::onLineClicked(){
  _stitcherView->setMode(StitcherView::Line);
}

void StitcherWorkspace::onArrowClicked(){
  _stitcherView->setMode(StitcherView::Default);
}

void StitcherWorkspace::onRotateClicked(){
  if(_stitcherView->selectedImage()){
    _stitcherView->selectedImage()->rotateImage();
  }
}


QTreeView * StitcherWorkspace::createGeometryTree(){
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  QTreeView *treeView = new QTreeView(this);
  geometryTree = treeView;
  treeView->setModel(model);
  loadGeometry();
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->resizeColumnToContents(0);
  treeView->resizeColumnToContents(1);
  connect(model,SIGNAL(itemChanged(QStandardItem * )),this,SLOT(onItemChanged(QStandardItem *)));
  return treeView;  
}

void StitcherWorkspace::loadGeometry(){
  /* 
     This is a prefix to distinguish Hawk Geometry properties from the
     normal widget properties 
  */
  const QString tag("HawkGeometry_");
  QList<QGraphicsItem *> ii  = _stitcherView->items();
  QMap<QString,ImageItem *> sortMap;
  for(int i = 0;i<ii.size();i++){
    if(ImageItem * imageItem = qgraphicsitem_cast<ImageItem *>(ii[i])){
      if(imageItem->isVisible()){
	sortMap.insert(imageItem->identifier(),imageItem);
      }
    }
  }
  QList<ImageItem *>sortedItems = sortMap.values();
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(geometryTree->model());
  model->clear();
  model->setHorizontalHeaderLabels(QStringList() << "Parameter" << "Value");
  
  for(int i = 0;i<sortedItems.size();i++){
    ImageItem * imageItem = sortedItems[i];
    const QMetaObject *metaobject =  imageItem->metaObject();
    int count = metaobject->propertyCount();
    QStandardItem * itemName = new QStandardItem(imageItem->identifier());
    QStandardItem * itemValue = new QStandardItem();
    QStandardItem *parentItem = model->invisibleRootItem();
    itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
    itemValue->setFlags(itemValue->flags() & ~Qt::ItemIsEditable);
    parentItem->appendRow(QList<QStandardItem *>() << itemName <<  itemValue);
    parentItem = itemName;
    for (int j=0; j<count; ++j) {
      QMetaProperty metaproperty = metaobject->property(j);
      const char *name = metaproperty.name();
      if(!QString(name).startsWith(tag)){
	continue;
      }
      QVariant var =  imageItem->property(name);
      Qt::ItemFlags itemValueFlags = Qt::ItemIsSelectable|Qt::ItemIsEnabled;
      if(metaproperty.isWritable()){
	itemValueFlags |= Qt::ItemIsEditable;
      }
      if(var.type() == QVariant::Double){
	double value = var.toDouble();
	QStandardItem * itemName = new QStandardItem(_stitcherView->propertyNameToDisplayName(name,tag));
	QStandardItem * itemValue = new QStandardItem(QString("%0").arg(value));
	itemValue->setData(value,Qt::UserRole + 1);
	itemValue->setData(QString(name),Qt::UserRole + 2);
	itemValue->setData(QVariant::fromValue(imageItem),Qt::UserRole + 3);
	itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
	itemValue->setFlags(itemValueFlags);
	parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
      }
    }
  }
  geometryTree->expandAll();
  geometryTree->sortByColumn(0,Qt::AscendingOrder);
}


void StitcherWorkspace::onItemChanged(QStandardItem * item){
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
  if(var.type() == QVariant::Double){
    double value = item->text().toDouble();
    QString property = item->data(Qt::UserRole + 2).toString();
    ImageItem * imageItem = item->data(Qt::UserRole + 3).value<ImageItem *>();
    item->setText(QString("%0").arg(value));		  
    imageItem->setProperty(property.toAscii().constData(),value);
    qDebug("New value for %s = %f",property.toAscii().data(),value);
    loadGeometry();
  }
}

