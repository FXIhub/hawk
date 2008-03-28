#include "imageViewer.h"
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QKeyEvent>
#include "imageItem.h"
#include "imageBay.h"

ImageViewer::ImageViewer(QGraphicsView * view,QWidget * parent)
  :QGraphicsScene(parent)
{
  graphicsView = view;
  dragged = 0;
  itemsScale.setX(1);
  itemsScale.setY(1);
}

void ImageViewer::mousePressEvent(QGraphicsSceneMouseEvent * event){
  //    qDebug("Mouse press");
  if(event->buttons() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->scenePos());
    for(int i = 0; i < it.size(); i++){
      if(QString("ImageItem") == it[i]->data(0)){
	dragged = (ImageItem *)it[i];
	draggedInitialPos = it[i]->mapFromScene(event->scenePos())-it[i]->pos();
	it[i]->setFocus(Qt::MouseFocusReason);
	break;
      }
    }
  }
}

void ImageViewer::mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent ){
  if(dragged){
    dragged = 0;
  }
}

void ImageViewer::mouseMoveEvent(QGraphicsSceneMouseEvent * event){
  //  qDebug("Mouse move");
  if(dragged && event->buttons() & Qt::LeftButton){
    QPointF mov = event->scenePos()-event->lastScenePos();
    dragged->moveBy(mov.x(),mov.y());
    return;
  }
  if(event->buttons() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items();
    for(int i = 0; i < it.size(); i++){
      if(!it[i]->parentItem()){
	QPointF mov = event->scenePos()-event->lastScenePos();
	it[i]->moveBy(mov.x(),mov.y());
      }
    }
  }else if(event->buttons() & Qt::RightButton){  
    QPointF mouse_mov = event->screenPos()-event->lastScreenPos();  
    qreal speed = 0.005;
    qreal scale = 1-mouse_mov.y()*speed;
    scaleItems(scale);
  }
  event->accept();
}
void ImageViewer::wheelEvent( QGraphicsSceneWheelEvent * event ){
  qreal speed = 0.0005;
  qreal scale = 1+event->delta()*speed;
  scaleItems(scale);
}


void ImageViewer::scaleItems(qreal scale){
  QPointF screen_center = graphicsView->sceneRect().center();
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

void ImageViewer::addImage(ImageItem * item){
  addItem(item);
  item->scale(itemsScale.x(),itemsScale.y());
  // Set pixmap center in the middle of the screen
  QPointF center = graphicsView->sceneRect().center();
  center.setX(center.x()-item->pixmap().width()*itemsScale.x()/2);
  center.setY(center.y()-item->pixmap().height()*itemsScale.y()/2);
  item->setPos(center);
}

void ImageViewer::keyReleaseEvent ( QKeyEvent * event ){
  if(event->key() == Qt::Key_Plus){
    scaleItems(1.25);    
  }
  if(event->key() == Qt::Key_Minus){
    scaleItems(0.75);    
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

  ImageBay * bay = new ImageBay(Qt::LeftDockWidgetArea,QString("Mask"),pen,brush,NULL);
  bayList.append(bay);
  addItem(bay);

  gradient = QLinearGradient();
  gradient.setColorAt(0,QColor("#6699cc"));
  gradient.setColorAt(1,QColor("#aaccee"));
  pen.setColor(QColor("#336699"));
  brush = QBrush(gradient);

  bay = new ImageBay(Qt::TopDockWidgetArea,QString("Input"),pen,brush,NULL);
  bayList.append(bay);
  addItem(bay);

  gradient = QLinearGradient();
  gradient.setColorAt(0,QColor("#f0a513"));
  gradient.setColorAt(1,QColor("#eec73e"));
  pen.setColor(QColor("#fb8b00"));
  brush = QBrush(gradient);

  bay = new ImageBay(Qt::BottomDockWidgetArea,QString("Output"),pen,brush,NULL);
  bayList.append(bay);
  addItem(bay);
  
  /*  QGraphicsTextItem * t  = new QGraphicsTextItem("test",0);
  t->setDefaultTextColor(QColor("#ffffff"));
  t->setAcceptsHoverEvents(true);
  addItem(t);*/
}
