#include "imageViewer.h"
#include <QtGui>
#include "imageItem.h"
#include "imageBay.h"
#include "mainwindow.h"

ImageViewer::ImageViewer(QWidget * parent)
  :QGraphicsView(parent)
{
  dragged = 0;
  itemsScale.setX(1);
  itemsScale.setY(1);
  mode = ModeDefault;
  controlDown = false;
}

void ImageViewer::mousePressEvent(QMouseEvent * event){
  //    qDebug("Mouse press");
  if(event->buttons() & Qt::LeftButton){
    if(getMode() == ModeDefault){
      QList<QGraphicsItem *> it = items(event->pos());
      for(int i = 0; i < it.size(); i++){
	if(QString("ImageItem") == it[i]->data(0)){
	  dragged = (ImageItem *)it[i];
	  draggedInitialPos = it[i]->mapFromScene(mapToScene(event->pos()))-it[i]->pos();
	  it[i]->setFocus(Qt::MouseFocusReason);
	  break;
	}
      }
      if(dragged){
	setCursor(QCursor(Qt::OpenHandCursor));
      }else{
	setCursor(QCursor(Qt::ClosedHandCursor));
      }

    }else if(getMode() == ModeExcludeFromMask || getMode() == ModeIncludeInMask){
      QPoint cursorHalfLength = QPoint(16,16);
      QPoint pos = event->pos();
      QList<QGraphicsItem *> it = items(event->pos());
      foreach(QGraphicsItem * item,it){
	if(QString("ImageItem") == item->data(0)){
	  ImageItem * ii = (ImageItem *)item;
	  if(ii->hasFocus()){
	    if(getMode() == ModeExcludeFromMask){
	      ii->excludeFromMask(QRectF(mapToScene(pos-cursorHalfLength),mapToScene(pos+cursorHalfLength)));
	    }else if(getMode() == ModeIncludeInMask){
	      ii->includeInMask(QRectF(mapToScene(pos-cursorHalfLength),mapToScene(pos+cursorHalfLength)));
	    }
	  }else{
	    ii->setFocus(Qt::MouseFocusReason);
	  }
	  break;
	}      
      }
    }else if(getMode() == ModePickCenter){
      QPoint cursorHalfLength = QPoint(16,16);
      QPoint pos = event->pos();
      QList<QGraphicsItem *> it = items(event->pos());
      foreach(QGraphicsItem * item,it){
	if(QString("ImageItem") == item->data(0)){
	  ImageItem * ii = (ImageItem *)item;
	  if(ii->hasFocus()){
	    ii->setImageCenter(mapToScene(event->pos()));
	  }else{
	    ii->setFocus(Qt::MouseFocusReason);
	  }

	  break;
	}      
      }
    }
  }
}

void ImageViewer::mouseReleaseEvent( QMouseEvent * mouseEvent ){
  if(dragged){
    dragged = 0;
  }
  setModeCursor();

}

void ImageViewer::mouseMoveEvent(QMouseEvent * event){
  //  qDebug("Mouse move");
  if(getMode() == ModeDefault){
    if(dragged && event->buttons() & Qt::LeftButton){
      QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
      dragged->moveBy(mov.x(),mov.y());
    }else if(event->buttons() & Qt::LeftButton){
      QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
      translateItems(mov);
    }else if(event->buttons() & Qt::RightButton){  
      QPointF mouse_mov = mapToScene(event->pos())-mouseLastScenePos;
      qreal speed = 0.005;
      qreal scale = 1-mouse_mov.y()*speed;
      scaleItems(scale);
    }
  }else if(event->buttons() & Qt::LeftButton && 
	   (getMode() == ModeExcludeFromMask || getMode() == ModeIncludeInMask)){
    QPoint cursorHalfLength = QPoint(16,16);
    QPoint pos = event->pos();
    QList<QGraphicsItem *> it = items(event->pos());
    foreach(QGraphicsItem * item,it){
      if(QString("ImageItem") == item->data(0)){
	ImageItem * ii = (ImageItem *)item;
	if(getMode() == ModeExcludeFromMask){
	  ii->excludeFromMask(QRectF(mapToScene(pos-cursorHalfLength),mapToScene(pos+cursorHalfLength)));
	}else if(getMode() == ModeIncludeInMask){
	  ii->includeInMask(QRectF(mapToScene(pos-cursorHalfLength),mapToScene(pos+cursorHalfLength)));
	}
	break;
      }      
    }
  }


  bool oldMouseInsideImage = mouseInsideImage;
  mouseInsideImage = false;
  QList<QGraphicsItem *> it = items(event->pos());
  for(int i = 0; i < it.size(); i++){
    if(QString("ImageItem") == it[i]->data(0)){
      mouseInsideImage = true;
      break;
    }
  }
  if(oldMouseInsideImage != mouseInsideImage){
    setModeCursor();
  }
  mouseLastScenePos = mapToScene(event->pos());
  event->accept();
}

void ImageViewer::wheelEvent( QWheelEvent * event ){
  qreal speed = 0.0005;
  qreal scale = 1+event->delta()*speed;
  scaleItems(scale);
}


void ImageViewer::scaleItems(qreal scale){
  // Don't let the user zoom in or out too much 
  if((scale < 1 && scale > 0 && 
      itemsScale.x() > 0.01 &&
      itemsScale.y() > 0.01) ||
     (scale > 1 && 
     itemsScale.x() < 100 &&
      itemsScale.y() < 100)){
    QPointF screen_center = sceneRect().center();
    QList<QGraphicsItem *> it = items();
    itemsScale.setX(itemsScale.x()*scale);
    itemsScale.setY(itemsScale.y()*scale);
    for(int i = 0; i < it.size(); i++){
      // Only scale the top level items     
      if(!it[i]->parentItem()){
	if(QString("ImageItem") == it[i]->data(0)){
	  ((ImageItem *)it[i])->centeredScale(scale,screen_center);
	}else if(QString("Bay") == it[i]->data(0)){
	  ((ImageBay *)it[i])->centeredScale(scale,screen_center);
	}else{
	  QPointF item_sc = it[i]->mapFromScene(screen_center);
	  it[i]->scale(scale, scale);
	  QPointF item_a_sc = it[i]->mapFromScene(screen_center);
	  QPointF mov = item_a_sc-item_sc;
	  it[i]->translate(mov.x(),mov.y());
	}
      }
    }
  }
}

void ImageViewer::translateItems(QPointF mov){
  QList<QGraphicsItem *> it = items();
  for(int i = 0; i < it.size(); i++){
    if(!it[i]->parentItem()){
      it[i]->moveBy(mov.x(),mov.y());
    }
  } 
}

void ImageViewer::addImage(ImageItem * item){
  imageItems.append(item);
  graphicsScene->addItem(item);
  item->scale(itemsScale.x(),itemsScale.y());
  // Set pixmap center in the middle of the screen
  QPointF center = sceneRect().center();
  center.setX(center.x()-item->pixmap().width()*itemsScale.x()/2);
  center.setY(center.y()-item->pixmap().height()*itemsScale.y()/2);
  item->setPos(center);
}

void ImageViewer::keyPressEvent ( QKeyEvent * event ){
  if(event->key() == Qt::Key_Plus){
    scaleItems(1.25);    
  }
  if(event->key() == Qt::Key_Minus){
    scaleItems(0.75);    
  }
  if(event->key() == Qt::Key_Up){
    translateItems(QPointF(0,10));
  }
  if(event->key() == Qt::Key_Down){
    translateItems(QPointF(0,-10));
  }
  if(event->key() == Qt::Key_Right){
    translateItems(QPointF(10,0));
  }

  if(event->key() == Qt::Key_Left){
    translateItems(QPointF(-10,0));
  }
  if(event->modifiers() & Qt::ControlModifier){
    controlDown = true;
    setModeCursor();
  }
}

void ImageViewer::keyReleaseEvent ( QKeyEvent * event ){
  if((event->modifiers() & Qt::ControlModifier) == 0){    
    controlDown = false;
    setModeCursor();
  }
}

void ImageViewer::createPreprocessBays(){
  QPen pen;
  QBrush brush;
  QLinearGradient gradient;
  pen.setWidthF(2);
  pen.setCosmetic(true);
  pen.setStyle(Qt::SolidLine);
  pen.setJoinStyle(Qt::RoundJoin);
  gradient.setColorAt(0,QColor("#d9bb7a"));
  gradient.setColorAt(1,QColor("#fdd99b"));
  pen.setColor(QColor("#816647"));
  brush = QBrush(gradient);
  ImageBay * bay;
  /*   bay = new ImageBay(Qt::LeftDockWidgetArea,QString("Mask"),pen,brush,NULL);
  bayList.append(bay);
  graphicsScene->addItem(bay);*/

  gradient = QLinearGradient();
  gradient.setColorAt(0,QColor("#6699cc"));
  gradient.setColorAt(1,QColor("#aaccee"));
  pen.setColor(QColor("#336699"));
  brush = QBrush(gradient);

  bay = new ImageBay(Qt::TopDockWidgetArea,QString("Input"),pen,brush,NULL);
  bayList.append(bay);
  graphicsScene->addItem(bay);

  gradient = QLinearGradient();
  gradient.setColorAt(0,QColor("#f0a513"));
  gradient.setColorAt(1,QColor("#eec73e"));
  pen.setColor(QColor("#fb8b00"));
  brush = QBrush(gradient);

  bay = new ImageBay(Qt::BottomDockWidgetArea,QString("Output"),pen,brush,NULL);
  bayList.append(bay);
  graphicsScene->addItem(bay);
}


void ImageViewer::setMode(ApplicationMode newMode){
  if(mode == ModeDefault){
    alternateMode = mode;
    mode = newMode;
  }else if(mode == ModeIncludeInMask || mode == ModeExcludeFromMask || mode == ModePickCenter){
    alternateMode = ModeDefault;
    mode = newMode;
  }  
  setModeCursor();
  foreach(ImageItem * image, imageItems){
    image->setMode(newMode);
  }
}

void ImageViewer::setModeCursor(){
  if(getMode() == ModeDefault){
    setCursor(QCursor(Qt::ArrowCursor));
  }else if(getMode() == ModeExcludeFromMask){
    if(mouseInsideImage){
      setCursor(QCursor(QPixmap(":/images/32x32/mask_exclude.png")));
    }else{
      setCursor(QCursor(Qt::ForbiddenCursor));
    }
  }else if(getMode() == ModeIncludeInMask){
    if(mouseInsideImage){
      setCursor(QCursor(QPixmap(":/images/32x32/mask_include.png")));
    }else{
      setCursor(QCursor(Qt::ForbiddenCursor));
    }
  }else if(getMode() == ModePickCenter){
    if(mouseInsideImage){
      setCursor(QCursor(Qt::CrossCursor));
    }else{
      setCursor(QCursor(Qt::ForbiddenCursor));
    }
  }
}

ApplicationMode ImageViewer::getMode(){
  if(QApplication::keyboardModifiers() & Qt::ControlModifier){
    return alternateMode;
  }
  return mode;
}

void ImageViewer::setup(MainWindow * main){
  mainWindow = main;
  setSceneRect(QRect(0,0,400,400));
  setBackgroundBrush(QBrush(QColor(Qt::black)));
  graphicsScene = new QGraphicsScene(this);
  setScene(graphicsScene);    
  createPreprocessBays();
  scaleItems(0.1);
  QObject::connect(mainWindow,SIGNAL(modeChanged(ApplicationMode)),this,SLOT(setMode(ApplicationMode)));
}

void ImageViewer::loadImage(QString filename){
  Image * image = sp_image_read(filename.toAscii(),0);
  if(!image){
    qDebug(("Failed to read image " + filename).toAscii());
    return;
  }
  ImageItem * item = new ImageItem(image,mainWindow,filename,NULL);
  item->setMode(mode);
  addImage(item);    
  item->update();
  item->setFocus(Qt::MouseFocusReason);
  setModeCursor();
}
