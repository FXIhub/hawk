#include "imageitem.h"
#include <QtGui>
#include <math.h>

ImageItem::ImageItem(Image * sp_image,QString file,QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
  filename = file;
  colormap_flags = COLOR_JET;
  colormap_min = 0;
  colormap_max = 0;
  image = sp_image;

  colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
  data = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
  /*  mask = QImage(sp_image_x(image),sp_image_y(image),QImage::Format_ARGB32);
      maskFaint = QImage(sp_image_x(image),sp_image_y(image),QImage::Format_ARGB32);*/
  /*  for(int x = 0;x<sp_image_x(image);x++){
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
    }*/
  setPixmap(QPixmap::fromImage(data));
  setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsFocusable);
  selectRect = 0;
  setData(0,QString("ImageItem"));
  setZValue(10);
  selectRect = new QGraphicsRectItem(0,0,pixmap().width(),pixmap().height(),this);
  selectRect->setVisible(false);
}


ImageItem::ImageItem(QPixmap pix,QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
  colormap_flags = COLOR_JET;
  setPixmap(pix);
  colormap_data = NULL;
  image = NULL;
  setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsFocusable);
}

QPointF ImageItem::centeredScale(qreal s,QPointF screenCenter){
  QPointF item_sc = mapFromScene(screenCenter);
  scale(s, s);
  QPointF mov = mapFromScene(screenCenter)-item_sc;
  translate(mov.x(),mov.y());
  return QPointF(transform().m11(),transform().m22());
}

ImageItem::~ImageItem()
{
  free(colormap_data);
  if(image){
    sp_image_free(image);
  }
}

void ImageItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
		      QWidget * widget){
  QGraphicsPixmapItem::paint(painter,option,widget);
  //  painter->drawImage(0,0,data);

  /*  if(mode == ModeExcludeFromMask || mode == ModeIncludeInMask){
    painter->drawImage(0,0,mask);
  }else{
    painter->drawImage(0,0,maskFaint);
    }*/
};

Image * ImageItem::getImage(){
  return image;
}

QPointF ImageItem::getScale(){
  return QPointF(transform().m11(),transform().m22());
}

void ImageItem::shiftImage(){
  if(image){
    Image * tmp = sp_image_shift(image);
    if(tmp){
      sp_image_free(image);
      image = tmp;
      colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
      data = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
      setPixmap(QPixmap::fromImage(data));
    }
  }
}

bool ImageItem::isShifted(){
  if(image){
    return image->shifted;
  }
  return false;
}

void ImageItem::setColormap(int color){
 // clear color bits
  if(image){
    colormap_flags &= ~(COLOR_GRAYSCALE|COLOR_TRADITIONAL|COLOR_HOT|COLOR_JET|COLOR_WHEEL|COLOR_RAINBOW);
    colormap_flags |= color;
    updateImage();
  }
}


void ImageItem::updateImage(){
  if(image){
    if(colormap_data){
      free(colormap_data);
    }
    colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
    data = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
    setPixmap(QPixmap::fromImage(data));
  }
}

int ImageItem::colormap(){
  return colormap_flags & (COLOR_GRAYSCALE|COLOR_TRADITIONAL|COLOR_HOT|COLOR_JET|COLOR_WHEEL|COLOR_RAINBOW|LOG_SCALE);
}

void ImageItem::setDisplay(int display){
 // clear display bits
  if(image){
    colormap_flags &= ~(COLOR_MASK|COLOR_PHASE);
    colormap_flags |= display;
    updateImage();
  }
}

int ImageItem::display(){
  return colormap_flags & (COLOR_MASK|COLOR_PHASE);
}

void ImageItem::maxContrast(QRectF area){
  // the area is given in scene coordinates. We have to change to item coordinates
  QPointF tl = mapFromScene(area.topLeft());
  QPointF br = mapFromScene(area.bottomRight());
  if(!image){
    return;
  }
  bool initialized = false;
  real min;
  real max;
  for(int x = (int)tl.x();x<=br.x();x++){
    if(x < 0 || x >= sp_image_x(image)){
      continue;
    }
    for(int y = (int)tl.y();y<=br.y();y++){
      if(y < 0 || y >= sp_image_y(image)){
	continue;
      }
      real value = sp_cabs(sp_image_get(image,x,y,0));
      if(!initialized){
	min = value;
	max = value;
	initialized = true;
      }else{
	if(value < min){
	  min = value;
	}
	if(value > max){
	  max = value;
	}
      }
    }
  }
  if(initialized){
    colormap_min = min;
    colormap_max = max;
    updateImage();
  }
}

void ImageItem::logScale(bool on){
  colormap_flags &= ~(LOG_SCALE);
  if(on){
    colormap_flags |= LOG_SCALE;    
  }
  updateImage();
}
