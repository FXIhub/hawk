#include "imageitem.h"
#include <QtGui>
#include <math.h>

ImageItem::ImageItem(Image * sp_image,QString file,QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
  filename = file;
  colormap_flags = SpColormapJet;
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
  colormap_flags = SpColormapJet;
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

void ImageItem::fourierTransform(QRectF area,bool squared){
  if(image){
    // the area is given in scene coordinates. We have to change to item coordinates
    QPointF tl = mapFromScene(area.topLeft());
    QPointF br = mapFromScene(area.bottomRight());
    int origin_x = sp_max(0,tl.x());
    int origin_y = sp_max(0,tl.y());
    int width = sp_min(sp_image_x(image),br.x()) - sp_max(0,tl.x());
    int height = sp_min(sp_image_y(image),br.y()) - sp_max(0,tl.y());
    Image * tmp = sp_image_alloc(width,height,1);
    for(int x = (int)tl.x();x<=br.x();x++){
      if(x < 0 || x >= sp_image_x(tmp)){
	continue;
      }
      for(int y = (int)tl.y();y<=br.y();y++){
	if(y < 0 || y >= sp_image_y(tmp)){
	  continue;
	}
	sp_image_set(tmp,x-origin_x,y-origin_y,0,sp_image_get(image,x,y,0));
      }
    }
    memcpy(tmp->detector,image->detector,sizeof(Detector));
    //    Image * tmp = sp_image_duplicate(image,SP_COPY_ALL);
    if(squared){
      for(int i= 0;i<sp_image_size(tmp);i++){
	tmp->image->data[i] = sp_cinit(sp_cabs2(tmp->image->data[i]),0);
      }
    }
    Image * tmp2 = sp_image_fft(tmp);
    sp_image_scale(tmp2,1.0/sqrt(sp_image_size(tmp2)));
    sp_image_free(tmp);
    tmp = tmp2;
    
    if(tmp && sp_image_is_valid(tmp)){
      sp_image_free(image);
      image = tmp;
      colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
      data = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
      setPixmap(QPixmap::fromImage(data));
    }else if(tmp){
      sp_image_free(tmp);
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
    colormap_flags &= ~(SpColormapLastColorScheme-1);
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
  return colormap_flags & (SpColormapLastColorScheme-1);
}

void ImageItem::setDisplay(int display){
 // clear display bits
  if(image){
    colormap_flags &= ~(SpColormapPhase|SpColormapMask);
    colormap_flags |= display;
    updateImage();
  }
}

int ImageItem::display(){
  return colormap_flags & (SpColormapPhase|SpColormapMask);
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

void ImageItem::setLogScale(bool on){
  colormap_flags &= ~(SpColormapLogScale);
  if(on){
    colormap_flags |= SpColormapLogScale;    
  }
  updateImage();
}

bool ImageItem::logScale(){
  colormap_flags &= ~(SpColormapLogScale);
  if(colormap_flags & SpColormapLogScale){
    return true;
  }
  return false;
}

