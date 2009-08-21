#include <QtGui>
#include "stitcherview.h"
#include "imageitem.h"
#include "imageviewpanel.h"

StitcherView::StitcherView(QWidget * parent)
  :ImageView(parent)
{
  imageViewPanel()->showSaveButton(true);
  setRenderHints(QPainter::Antialiasing);
  QGraphicsLineItem * centerVerticalIndicator = new QGraphicsLineItem(0,-100000,0,100000);
  QGraphicsLineItem * centerHorizontalIndicator = new QGraphicsLineItem(-100000,0,100000,0);
  centerVerticalIndicator->setZValue(11);
  centerHorizontalIndicator->setZValue(11);
  graphicsScene->addItem(centerVerticalIndicator);
  graphicsScene->addItem(centerHorizontalIndicator);
  QPen pen = centerHorizontalIndicator->pen();  
  pen.setColor(Qt::white);
  pen.setStyle(Qt::SolidLine);
  centerHorizontalIndicator->setPen(pen);
  centerVerticalIndicator->setPen(pen);

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
  if(selectedImage()){
    // and the zoom
    item->setTransform(selectedImage()->transform());
    /* make sure the center is in the center */
    //    QSizeF center_correction = selectedImage()->boundingRect().size()/2-item->boundingRect().size()/2;
    item->setPos(-item->imageCenter()/2);

    // and colormap and display
    color = selectedImage()->colormap();
    display = selectedImage()->display();
    isShifted = selectedImage()->isShifted();
  }else{
    _selected = item;
    // Set pixmap center in the middle of the screen
    QPointF center = sceneRect().center();
    center.setX(center.x()-item->pixmap().width()*item->getScale().x()/2);
    center.setY(center.y()-item->pixmap().height()*item->getScale().y()/2);
    item->setPos(center);
  }
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
  emit imageItemChanged(item);
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
  }else if(event->button() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      if(QString("ImageItem") == it[i]->data(0)){
	if(selectedImage()){
	  selectedImage()->setSelected(false);
	}
	_selected = qgraphicsitem_cast<ImageItem *>(it[i]);
	selectedImage()->setSelected(true);	
      }
    }
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

void StitcherView::mouseMoveEvent(QMouseEvent * event){
  if(mode == Line || mode == Circle){
    lineEnd = event->pos();
    scene()->update();
  }else{
    ImageView::mouseMoveEvent(event);  
  }
}

void StitcherView::mousePressEvent( QMouseEvent *  event){
  if(mode == Line || mode == Circle){
    lineOrigin = event->pos();
  }else{
    ImageView::mousePressEvent(event);
  }
}

void StitcherView::clearHelpers(){
  QList<QGraphicsItem *> it = items();
  for(int i = 0; i < it.size(); i++){
    if(QString("Helper") == it[i]->data(0)){
      graphicsScene->removeItem(it[i]);
    }  
  }
}
