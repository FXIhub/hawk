#include "imageItem.h"
#include <QtGui>
#include <math.h>
#include "mainwindow.h"

ImageItem::ImageItem(Image * sp_image,MainWindow * main,QString file,QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
  filename = file;
  mainWindow = main;
  colormap_flags = COLOR_JET;
  colormap_min = 0;
  colormap_max = 0;
  image = sp_image;
  colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
  data = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
  mask = QImage(sp_image_x(image),sp_image_y(image),QImage::Format_ARGB32);
  maskFaint = QImage(sp_image_x(image),sp_image_y(image),QImage::Format_ARGB32);
  sniperScope = QImage(":/images/64x64/sniper_scope.png");
  for(int x = 0;x<sp_image_x(image);x++){
    for(int y = 0;y<sp_image_y(image);y++){
      int m = sp_i3matrix_get(image->mask,x,y,0);
      if(m == 1){
	mask.setPixel(x,y,0x44000000U);
	maskFaint.setPixel(x,y,0x00000000U);
      }else if(m == 0){
	mask.setPixel(x,y,0xFF000000U);
	maskFaint.setPixel(x,y,0x66000000U);
      }
    }
  }
  setPixmap(QPixmap::fromImage(data));
  setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsFocusable);
  selectRect = 0;
  setData(0,QString("ImageItem"));
  setZValue(10);
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event){

  /*  if(event->buttons() & Qt::LeftButton){
    QPointF mouse_mov = event->scenePos()-event->lastScenePos();  
    QTransform init = this->transform();
  // Take into account the zoom of the item 
    QLine l = init.map(QLine(1,0,0,0));
    qreal scale = sqrt(l.dx()*l.dx()+l.dy()+l.dy());
    QTransform move = QTransform().translate(mouse_mov.x()/scale, mouse_mov.y()/scale);    
    this->setTransform(init*QTransform().translate(mouse_mov.x(), mouse_mov.y()));
   }
  */
  
  if(event->buttons() & Qt::RightButton){
    qDebug("Right button drag on item");
  }


}
void ImageItem::wheelEvent( QGraphicsSceneWheelEvent * event ){
  qDebug("Mouse Wheel");
  QPointF scene_pos = (event->scenePos());
  QPointF mouse_pos = mapFromScene(scene_pos);
  qreal speed = 0.005;
  qreal scale = 1+event->delta()*speed;
  QTransform trans = this->transform();
  qDebug("pos %f %f scene pos %f %fscale %f",mouse_pos.x(),mouse_pos.y(),scene_pos.x(),scene_pos.y(),scale);
  this->setTransform(trans.scale(scale, scale).translate(mouse_pos.x(), mouse_pos.y()).translate(-mouse_pos.x()*scale,-mouse_pos.y()*scale));
}

void ImageItem::keyReleaseEvent ( QKeyEvent * event ){
  if(event->matches(QKeySequence::Delete) ||
     event->matches(QKeySequence::Cut) ||
     event->key() == Qt::Key_Backspace){
    // I don't know if this is safe;
    this->scene()->removeItem(this);    
  }  
}

void ImageItem::select(){
  selectRect = new QGraphicsRectItem(0,0,pixmap().width(),pixmap().height(),this);
  QPen p = QPen(QBrush(Qt::gray),1,Qt::SolidLine,Qt::RoundCap, Qt::RoundJoin);
  p.setCosmetic(true);
  selectRect->setPen(p);
  mainWindow->setSelectedImageItem(this);
  setZValue(11);
}

void ImageItem::deselect(){
  if(selectRect){
    delete selectRect;
  }
  mainWindow->setSelectedImageItem(NULL);
  setZValue(10);
}

void ImageItem::focusInEvent ( QFocusEvent * event ){
  select();
}

void ImageItem::focusOutEvent ( QFocusEvent * event ){
  deselect();
}

QPointF ImageItem::centeredScale(qreal s,QPointF screenCenter){
  QPointF item_sc = mapFromScene(screenCenter);
  scale(s, s);
  QPointF mov = mapFromScene(screenCenter)-item_sc;
  translate(mov.x(),mov.y());
  //  if(selectRect){
    //    QPen pen = selectRect->pen();
    // constant width pen regardless of zoom
    //    pen.setWidthF(1.0/transform().m11());
    //    selectRect->setPen(pen);
  //  }
  return QPointF(transform().m11(),transform().m22());
}

ImageItem::~ImageItem()
{
  free(colormap_data);
  sp_image_free(image);
  //  QGraphicsPixmapItem::~QGraphicsPixmapItem();
}

void ImageItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
		      QWidget * widget){
  painter->drawImage(0,0,data);
  if(mode == ModeExcludeFromMask || mode == ModeIncludeInMask){
    painter->drawImage(0,0,mask);
  }else{
    painter->drawImage(0,0,maskFaint);
  }
  if(mode == ModePickCenter){
    // We have to check out for the border 
    QPointF centerAtScene = mapToScene(QPointF(image->detector->image_center[0],image->detector->image_center[1]));
    QRectF unscaledSize(mapFromScene(centerAtScene-QPointF(sniperScope.width()/2,sniperScope.height()/2)),
			mapFromScene(centerAtScene+QPointF(sniperScope.width()/2,sniperScope.height()/2)));
    QRectF target(QRectF(0,0,sp_image_x(image),sp_image_y(image)).intersected(unscaledSize));
    QRectF source(mapToScene(target.topLeft())-centerAtScene+QPointF(sniperScope.width()/2,sniperScope.height()/2),
		  mapToScene(target.bottomRight())-centerAtScene+QPointF(sniperScope.width()/2,sniperScope.height()/2));
    painter->drawImage(target,sniperScope,source);
  }
  
};

void ImageItem::setMode(ApplicationMode newMode){
  mode = newMode;
  update();
}

void ImageItem::excludeFromMask(QRectF area){
  QPointF topLeft = mapFromScene(area.topLeft());
  QPointF bottomRight = mapFromScene(area.bottomRight());
  for(int x = qMax(round(topLeft.x()),0.0);x<qMin(round(bottomRight.x()),(double)sp_image_x(image));x++){
    for(int y = qMax(round(topLeft.y()),0.0);y<qMin(round(bottomRight.y()),(double)sp_image_y(image));y++){
      sp_i3matrix_set(image->mask,x,y,0,0);
      mask.setPixel(x,y,0xFF000000U);
      maskFaint.setPixel(x,y,0x66000000U);
    }
  }
  update();
}


void ImageItem::includeInMask(QRectF area){
  QPointF topLeft = mapFromScene(area.topLeft());
  QPointF bottomRight = mapFromScene(area.bottomRight());
  for(int x = qMax(round(topLeft.x()),0.0);x<qMin(round(bottomRight.x()),(double)sp_image_x(image));x++){
    for(int y = qMax(round(topLeft.y()),0.0);y<qMin(round(bottomRight.y()),(double)sp_image_y(image));y++){
      sp_i3matrix_set(image->mask,x,y,0,1);
      mask.setPixel(x,y,0x44000000U);
      maskFaint.setPixel(x,y,0x00000000U);
    }
  }
  update();
}

Image * ImageItem::getImage(){
  return image;
}

void ImageItem::setImageCenter(QPointF scenePos){
  QPointF p= mapFromScene(scenePos);
  image->detector->image_center[0] = round(p.x());
  image->detector->image_center[1] = round(p.y());
  mainWindow->imageItemChanged(this);
  update();
}
