#include <QtGui>
#include "stitcherview.h"
#include "imageitem.h"
#include "imageviewpanel.h"

StitcherView::StitcherView(QWidget * parent)
  :ImageView(parent)
{
  imageViewPanel()->showSaveButton(true);
  setRenderHints(QPainter::Antialiasing);
  /*  QGraphicsLineItem * centerVerticalIndicator = new QGraphicsLineItem(0,-100000,0,100000);
  QGraphicsLineItem * centerHorizontalIndicator = new QGraphicsLineItem(-100000,0,100000,0);
  centerVerticalIndicator->setZValue(11);
  centerHorizontalIndicator->setZValue(11);
  graphicsScene->addItem(centerVerticalIndicator);
  graphicsScene->addItem(centerHorizontalIndicator);
  QPen pen = centerHorizontalIndicator->pen();  
  pen.setColor(Qt::white);
  pen.setStyle(Qt::SolidLine);
  centerHorizontalIndicator->setPen(pen);
  centerVerticalIndicator->setPen(pen);*/
  _showIdentifiers = true;
  setBackgroundDraggable(false);
}

bool StitcherView::loadImage(QString s){
  Image * a =  sp_image_read(s.toAscii(),0);
  ImageItem * item = new ImageItem(a,s,this,NULL);
  addImage(item);    
  item->update();
  return true;
}


void StitcherView::addImage(ImageItem * item){
  int display = -1;
  int color = -1;
  bool isShifted = false;
  /* Always add in the same position */
  _selected = item;
  item->setPos(-item->pixmap().width()/2,-item->pixmap().height()/2);
  if(preservesShift() && isShifted != item->isShifted()){
    item->shiftImage();
  }
  if(color >= 0){
    item->setColormap(color);
  }
  if(display >= 0){
    item->setDisplay(display);
  }
  graphicsScene->addItem(item);  
  item->showIdentifier(_showIdentifiers);
  emit imageItemChanged(item);
  emit imageItemGeometryChanged(item);
}

void StitcherView::mouseReleaseEvent( QMouseEvent *  event){
  ImageView::mouseReleaseEvent(event);
  if(mode == Line && event->button() & Qt::LeftButton){
    setMode(Default);
    QGraphicsLineItem * line = new QGraphicsLineItem(QLineF(mapToScene(lineOrigin),mapToScene(lineEnd)));
    line->setData(0,QString("Helper"));
    line->setZValue(11);    
    QPen pen = line->pen();  
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    line->setPen(pen);
    graphicsScene->addItem(line);    
  }else if(mode == Circle && event->button() & Qt::LeftButton){
    setMode(Default);
    QPointF lineOriginF = mapToScene(lineOrigin);
    QPointF lineEndF = mapToScene(lineEnd);
    QPointF circleCenter = (lineOriginF+lineEndF)/2;    
    qreal circleRadius = sqrt((lineOriginF-lineEndF).x()* (lineOriginF-lineEndF).x()+
			      (lineOriginF-lineEndF).y()* (lineOriginF-lineEndF).y())/2;
    QGraphicsEllipseItem * circle = new QGraphicsEllipseItem(QRect(circleCenter.x()-circleRadius,circleCenter.y()-circleRadius,circleRadius*2,circleRadius*2));
    circle->setData(0,QString("Helper"));
    circle->setZValue(11);    
    QPen pen = circle->pen();  
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    circle->setPen(pen);
    graphicsScene->addItem(circle);    
  }
}



void StitcherView::saveImage(){
  if(selectedImage() && selectedImage()->getImage()){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"));
    qDebug("Trying to save %s",fileName.toAscii().data());
    if(!fileName.isEmpty()){
      sp_image_write(selectedImage()->getImage(),fileName.toAscii().data(),0);
    }
  }
}

void StitcherView::setMode(Mode m){
  mode = m;
  if(m == Line || m == Circle){
    setCursor(Qt::CrossCursor);
  }
  if(m == Default){
    setCursor(Qt::ArrowCursor);
  }
}

void StitcherView::paintEvent(QPaintEvent *event){
  ImageView::paintEvent(event);
  QPainter p(viewport());
  if(mode == Line && QApplication::mouseButtons() & Qt::LeftButton){
    /* paint line out */
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawLine(lineOrigin,lineEnd);
  }else if(mode == Circle && QApplication::mouseButtons() & Qt::LeftButton){
    QPointF circleCenter = (lineOrigin+lineEnd)/2;    
    qreal circleRadius = sqrt((lineOrigin-lineEnd).x()* (lineOrigin-lineEnd).x()+
			      (lineOrigin-lineEnd).y()* (lineOrigin-lineEnd).y())/2;
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawEllipse(QRectF(circleCenter.x()-circleRadius,circleCenter.y()-circleRadius,circleRadius*2,circleRadius*2));
  }  

}

void StitcherView::mousePressEvent( QMouseEvent *  event){
  if(mode == Line || mode == Circle){
    lineOrigin = event->pos();
  }else{
    if(event->button() & Qt::LeftButton){
      QList<QGraphicsItem *> it = items(event->pos());
      for(int i = 0; i < it.size(); i++){
	if(QString("ImageItem") == it[i]->data(0)){
	  if(selectedImage()){
	    selectedImage()->setSelected(false);
	  }
	  _selected = qgraphicsitem_cast<ImageItem *>(it[i]);
	  selectedImage()->setSelected(true);	
	  break;
	}
      }
    }
    ImageView::mousePressEvent(event);
  }
}

void StitcherView::mouseMoveEvent(QMouseEvent * event){
  if(mode == Line || mode == Circle){
    lineEnd = event->pos();
    scene()->update();
  } else if(dragged && event->buttons() & Qt::LeftButton){
    QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
    dragged->moveBy(mov.x(),mov.y());
    emit imageItemGeometryChanged(dragged);
  }else if(event->buttons() & Qt::LeftButton){
    QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
    emit translateBy(mov);
  }else if(event->buttons() & Qt::RightButton){  
    QPointF mouse_mov = mapToScene(event->pos())-mouseLastScenePos;
    qreal speed = 0.005;
    qreal scale = 1-mouse_mov.y()*speed;
    emit scaleBy(scale);
  }
  mouseOverValue(event);
  mouseLastScenePos = mapToScene(event->pos());
  event->accept();
}

void StitcherView::clearHelpers(){
  QList<QGraphicsItem *> it = items();
  for(int i = 0; i < it.size(); i++){
    if(QString("Helper") == it[i]->data(0)){
      graphicsScene->removeItem(it[i]);
    }  
  }
}

void StitcherView::scaleItems(qreal new_scale){
  QList<QGraphicsItem *> it = items();
  for(int i = 0; i < it.size(); i++){
    if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it[i])){
      item->setDz(item->dz()*new_scale);
      emit imageItemGeometryChanged(item);
    }
  }  
}
