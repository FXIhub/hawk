#include <QtGui>
#include "stitcherworkspace.h"
#include "stitcherview.h"
#include "imageitem.h"

StitcherWorkspace::StitcherWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  _stitcherView = new StitcherView(this);    
  hbox->addWidget(createToolBar());
  hbox->addWidget(_stitcherView); 
}

QWidget * StitcherWorkspace::createToolBar(){
  QGroupBox * ret = new QGroupBox(this);
  ret->setTitle(tr("Tools"));
  QGridLayout * layout = new QGridLayout(ret);
  ret->setLayout(layout);
  QSize iconSize = QSize(22,22);
  QToolButton * stitch = new QToolButton(this);
  stitch->setIcon(QIcon(":images/stitch.png"));
  stitch->setToolTip(tr("Combine images in a single one mantaining relative position"));
  stitch->setIconSize(iconSize);
  connect(stitch,SIGNAL(clicked(bool)),this,SLOT(onStitchClicked()));
  layout->addWidget(stitch,0,0);

  QToolButton * line = new QToolButton(this);
  line->setIcon(QIcon(":images/add_line.png"));
  line->setToolTip(tr("Draw guide line"));
  line->setIconSize(iconSize);
  connect(line,SIGNAL(clicked(bool)),this,SLOT(onLineClicked()));
  layout->addWidget(line,1,0);

  QToolButton * circle = new QToolButton(this);
  circle->setIcon(QIcon(":images/add_circle.png"));
  circle->setToolTip(tr("Draw guide circle"));
  circle->setIconSize(iconSize);
  connect(circle,SIGNAL(clicked(bool)),this,SLOT(onCircleClicked()));
  layout->addWidget(circle,2,0);

  QToolButton * rotate = new QToolButton(this);
  rotate->setIcon(QIcon(":images/undo.png"));
  rotate->setToolTip(tr("Rotate image 180 degrees"));
  rotate->setIconSize(iconSize);
  connect(rotate,SIGNAL(clicked(bool)),this,SLOT(onRotateClicked()));
  layout->addWidget(rotate,3,0);

  QToolButton * clear = new QToolButton(this);
  clear->setIcon(QIcon(":images/clear.png"));
  clear->setToolTip(tr("Remove helper lines"));
  clear->setIconSize(iconSize);
  connect(clear,SIGNAL(clicked(bool)),_stitcherView,SLOT(clearHelpers()));
  layout->addWidget(clear,4,0);
 
 

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


void StitcherWorkspace::onLineClicked(){
  _stitcherView->setMode(StitcherView::Line);
}


void StitcherWorkspace::onCircleClicked(){
  _stitcherView->setMode(StitcherView::Circle);
}

void StitcherWorkspace::onRotateClicked(){
  if(_stitcherView->selectedImage()){
    _stitcherView->selectedImage()->rotateImage();
  }
}

