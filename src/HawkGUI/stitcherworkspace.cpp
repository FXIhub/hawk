#include <QtGui>
#include "stitcherworkspace.h"
#include "stitcherview.h"
#include "imageitem.h"
#include "addconstraintdialog.h"
#include "addvariabledialog.h"


Q_DECLARE_METATYPE(ImageItem *);
Q_DECLARE_METATYPE(AddConstraintDialog *);
StitcherWorkspace::StitcherWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  QSplitter * centerSplitter = new QSplitter(Qt::Horizontal,this);
  this->setLayout(hbox);
  hbox->addWidget(centerSplitter);
  _stitcherView = new StitcherView(this);  
  _stitcherView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  centerSplitter->addWidget(leftSplitter);
  leftSplitter->addWidget(createToolBar());
  leftSplitter->addWidget(createGeometryTree());
  leftSplitter->addWidget(createConstraintsTree());
  connect(_stitcherView,SIGNAL(imageItemGeometryChanged(ImageItem *)),this,SLOT(loadGeometry()));
  centerSplitter->addWidget(_stitcherView); 
  centerSplitter->setStretchFactor(0,0);
  centerSplitter->setStretchFactor(1,2);
  //  centerSplitter->setSizes(QList<int>() << 1 << 1000);
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
  layout->addWidget(stitch,0,3);

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

  QToolButton * clear = new QToolButton(this);
  clear->setIcon(QIcon(":images/clear.png"));
  clear->setToolTip(tr("Remove helper lines"));
  clear->setIconSize(iconSize);
  connect(clear,SIGNAL(clicked(bool)),_stitcherView,SLOT(clearHelpers()));
  layout->addWidget(clear,1,2);

  QToolButton * clearAll = new QToolButton(this);
  clearAll->setIcon(QIcon(":images/edit-delete.png"));
  clearAll->setToolTip(tr("Clear workspace"));
  clearAll->setIconSize(iconSize);
  connect(clearAll,SIGNAL(clicked(bool)),_stitcherView,SLOT(clearAll()));
  layout->addWidget(clearAll,1,3);


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
  model->setHorizontalHeaderLabels(QStringList() << "Parameter" << "Value" << "Locked");
  
  for(int i = 0;i<sortedItems.size();i++){
    ImageItem * imageItem = sortedItems[i];
    const QMetaObject *metaobject =  imageItem->metaObject();
    int count = metaobject->propertyCount();
    QStandardItem * itemName = new QStandardItem(imageItem->identifier());
    QStandardItem * itemValue = new QStandardItem();
    QStandardItem * itemLocked = new QStandardItem();
    itemLocked->setFlags(itemLocked->flags() & ~Qt::ItemIsEditable);
    QStandardItem *parentItem = model->invisibleRootItem();
    itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
    itemValue->setFlags(itemValue->flags() & ~Qt::ItemIsEditable);
    if(model->findItems(itemName->text()).empty()){
      parentItem->appendRow(QList<QStandardItem *>() << itemName <<  itemValue << itemLocked);
      parentItem = itemName;
    }else{
      parentItem = model->findItems(itemName->text()).first();
    }
    for (int j=0; j<count; ++j){
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
	QStandardItem * itemLocked = new QStandardItem();
	itemLocked->setFlags(itemLocked->flags() & ~Qt::ItemIsEditable);
	/* check for lock property */
	QString lockedPropertyName = name + QString("_locked");
	if(imageItem->property(lockedPropertyName.toAscii().data()).isValid()){
	  bool locked = imageItem->property(lockedPropertyName.toAscii().data()).toBool();
	  itemLocked->setCheckable(true);	
	  itemValue->setData(locked,Qt::UserRole + 1);
	  itemLocked->setData(QString(lockedPropertyName),Qt::UserRole + 2);
	  itemLocked->setData(QVariant::fromValue(imageItem),Qt::UserRole + 3);
	  if(locked){
	    itemLocked->setCheckState(Qt::Checked);
	    itemValue->setEnabled(false);
	  }
	}
	parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue << itemLocked);
      }
    }
  }
  geometryTree->expandAll();
  geometryTree->resizeColumnToContents(0);
  geometryTree->sortByColumn(0,Qt::AscendingOrder);
}


void StitcherWorkspace::onItemChanged(QStandardItem * item){
  if(item->isEnabled() == false){
    return;
  }
  if(item->flags() & Qt::ItemIsUserCheckable){
    bool value = (item->checkState() == Qt::Checked);
    QString property = item->data(Qt::UserRole + 2).toString();
    ImageItem * imageItem = item->data(Qt::UserRole + 3).value<ImageItem *>();
    imageItem->setProperty(property.toAscii().constData(),value);
    loadGeometry();
  }else if(item->flags() & Qt::ItemIsEditable){
    qDebug("item changed");
    QVariant var = item->data();
    if(var.type() == QVariant::Double){
      double value = item->text().toDouble();
      QString property = item->data(Qt::UserRole + 2).toString();
      ImageItem * imageItem = item->data(Qt::UserRole + 3).value<ImageItem *>();
      //      item->setText(QString("%0").arg(value));		  
      imageItem->setProperty(property.toAscii().constData(),value);
      qDebug("New value for %s = %f",property.toAscii().data(),value);
      loadGeometry();
    }
  }
}


QWidget * StitcherWorkspace::createConstraintsTree(){
  QWidget * top = new QWidget(this);
  QVBoxLayout * vbox = new QVBoxLayout(top);
  top->setContentsMargins(0,0,0,0);
  vbox->setContentsMargins(0,0,0,0);
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  QTreeView *treeView = new QTreeView(top);
  vbox->addWidget(treeView);
  constraintsTree = treeView;
  treeView->setModel(model);
  initConstraintsTree();
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->resizeColumnToContents(0);
  treeView->resizeColumnToContents(1);
  QPushButton * addConstraint = new QPushButton("Add Constraint",top);
  connect(addConstraint,SIGNAL(clicked()),this,SLOT(onAddConstraintClicked()));
  vbox->addWidget(addConstraint);
  QPushButton * optimizeGeometry = new QPushButton("Optimize Geometry",top);
  connect(optimizeGeometry,SIGNAL(clicked()),this,SLOT(onOptimizeGeometryClicked()));
  vbox->addWidget(optimizeGeometry);
  return top;
}


void StitcherWorkspace::initConstraintsTree(){
  /* 
     This is a prefix to distinguish Hawk Geometry properties from the
     normal widget properties 
  */
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(constraintsTree->model());
  model->clear();
  model->setHorizontalHeaderLabels(QStringList() << "Constraint" << "Error");
  constraintsTree->setColumnWidth(0,20);
}


void StitcherWorkspace::onAddConstraintClicked(){
  /* 
     This is a prefix to distinguish Hawk Geometry properties from the
     normal widget properties 
  */
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(constraintsTree->model());
  AddConstraintDialog * d = new AddConstraintDialog(_stitcherView);
  if(d->exec()){
    QList<QPair<int,ImageItem *> > points = d->selectedPoints();
    QStandardItem *parentItem = model->invisibleRootItem();
    QStandardItem * itemName = new QStandardItem("Type");
    itemName->setData(QVariant::fromValue(d));
    QStandardItem * itemValue;
    if(d->constraintType() == RadialLineConstraint){
      itemValue = new QStandardItem("Radial Line");
    }else{
      itemValue = new QStandardItem("Centered Circle");
    }
    parentItem->appendRow(QList<QStandardItem *>() << itemName <<  itemValue);
    parentItem = itemName;
    for(int i = 0;i<points.size();i++){

      itemName = new QStandardItem(points[i].second->identifier() + "." + QString::number(points[i].first));
      itemValue = new QStandardItem("");
      parentItem->appendRow(QList<QStandardItem *>() << itemName <<  itemValue);
    }
    itemName = new QStandardItem("Best Fit");
    itemValue = new QStandardItem("");
    parentItem->appendRow(QList<QStandardItem *>() << itemName <<  itemValue);

    
  }
  constraintsTree->expandAll();
  constraintsTree->resizeColumnToContents(0);
  constraintsTree->resizeColumnToContents(1);
}

void StitcherWorkspace::onAddVariableClicked(){
  /* 
     This is a prefix to distinguish Hawk Geometry properties from the
     normal widget properties 
  */
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(constraintsTree->model());
  AddVariableDialog * d = new AddVariableDialog;
  
}

void StitcherWorkspace::onOptimizeGeometryClicked(){
  geometrically_constrained_system * gc = geometrically_constrained_system_alloc();
  
  /* create positioned images */
  QMap<ImageItem *, positioned_image *> pos_image_map;
  QList<QGraphicsItem *> graphicsItems = _stitcherView->items();
  for(int i = 0; i < graphicsItems.size(); i++){
    if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(graphicsItems[i])){
      positioned_image * p = create_positioned_image(item->getImage());
      set_image_position(p,DeltaX,item->dx());
      if(!item->dxLocked()){
	geometrically_constrained_system_add_variable(gc,create_geometry_variable(p,DeltaX));
      }
      set_image_position(p,DeltaY,item->dy());
      if(!item->dyLocked()){
	geometrically_constrained_system_add_variable(gc,create_geometry_variable(p,DeltaY));
      }
      set_image_position(p,Zoom,50.0/item->dz());
      if(!item->dzLocked()){
	geometrically_constrained_system_add_variable(gc,create_geometry_variable(p,Zoom));
      }

      set_image_position(p,Theta,item->theta());
      if(!item->thetaLocked()){
	geometrically_constrained_system_add_variable(gc,create_geometry_variable(p,Theta));
      }

      pos_image_map.insert(item,p);
    }
  }


  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(constraintsTree->model());
  for(int i = 0;i<model->rowCount();i++){
    QStandardItem * it = model->item(i,0);
    AddConstraintDialog * d = it->data().value<AddConstraintDialog *>();
    geometric_constraint c =  geometric_constraint_init(d->constraintType(),0);
    

    QList<QPair<int,ImageItem *> > points = d->selectedPoints();
    for(int i = 0;i<points.size();i++){
      ImageItem * item = points[i].second;
      QPointF pos  = item->getControlPoints()[points[i].first];
      positioned_image * a = pos_image_map.value(item);
      control_point cp = create_control_point(a,pos.x()-sp_image_x(a->image)/2,sp_image_y(a->image)/2-pos.y());
      geometric_constraint_add_point(&c,cp);            
    }
    geometrically_constrained_system_add_constraint(gc,c);
  }
  if(model->rowCount()){
    geometry_contraint_minimizer(gc);  
  }
  _stitcherView->clearConstraintFits();
  for(int i = 0;i<model->rowCount();i++){
    QStandardItem * it = model->item(i,0);
    for(int j = 0;j<it->rowCount()-1;j++){
      it->child(j,1)->setText(QString("%0").arg(gc->constraints[i].error[j]));      
    }
    it->child(it->rowCount()-1,1)->setText(QString("%0").arg(gc->constraints[i].best_fit*180.0/M_PI));      	
    _stitcherView->drawConstraintFit(gc->constraints[i].best_fit,gc->constraints[i].type);
  }  
  for(int i = 0;i<gc->n_variables;i++){
    ImageItem * item = pos_image_map.keys(gc->variables[i].parent).first();
    if(gc->variables[i].type == DeltaX){
      item->setDx(gc->variables[i].parent->pos[DeltaX]);      
    }
    if(gc->variables[i].type == DeltaY){
      item->setDy(gc->variables[i].parent->pos[DeltaY]);      
    }
    if(gc->variables[i].type == Zoom){
      item->setDz(50.0/gc->variables[i].parent->pos[Zoom]);      
    }
    if(gc->variables[i].type == Theta){
      item->setTheta(50.0/gc->variables[i].parent->pos[Theta]);      
    }    
  }
  loadGeometry();
}
