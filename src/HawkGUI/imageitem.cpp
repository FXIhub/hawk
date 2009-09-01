#include "imageitem.h"
#include <QtGui>
#include <math.h>
#include "xcam.h"

Q_DECLARE_METATYPE(Image *);
Q_DECLARE_METATYPE(const Image *);


ImageItem::ImageItem(Image * sp_image,QString file, ImageView * view,QGraphicsItem * parent)
  :QObject(view),QGraphicsPixmapItem(parent)
{ 
  _view = view;
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
  selectRect->setPen(QPen(Qt::white));
  selectRect->setVisible(false);
  centerVerticalIndicator = new QGraphicsLineItem(0,-100000,0,100000,this);
  centerHorizontalIndicator = new QGraphicsLineItem(-100000,0,100000,0,this);
  centerVerticalIndicator->hide();
  centerHorizontalIndicator->hide();
  QPen pen = centerHorizontalIndicator->pen();
  
  pen.setColor(Qt::white);
  pen.setStyle(Qt::SolidLine);
  centerHorizontalIndicator->setPen(pen);
  centerVerticalIndicator->setPen(pen);
  repositionCenterIndicators();
  identifierString = _view->imageItemIdentifier(this);
  identifierItem = new QGraphicsTextItem(identifierString,this);
  identifierItem->setDefaultTextColor(Qt::white);
  qreal posy = -(identifierItem->boundingRect()).height();
  identifierItem->setPos(boundingRect().width()/2-(identifierItem->boundingRect()).width()/2,posy);
}


ImageItem::ImageItem(QPixmap pix,ImageView * view, QGraphicsItem * parent)
  :QGraphicsPixmapItem(parent)
{ 
  _view = view;
  colormap_flags = SpColormapJet;
  setPixmap(pix);
  colormap_data = NULL;
  image = NULL;
  setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsFocusable);
  identifierItem = NULL;
  identifierString = QString();
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

const Image * ImageItem::getImage(){
  const Image * ret = image;
  return ret;
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
    Image * tmp = sp_image_duplicate(image,SP_COPY_DETECTOR);
    sp_image_realloc(tmp,width,height,1);
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
    if(squared){
      for(int i= 0;i<sp_image_size(tmp);i++){
	sp_image_set_by_index(tmp,i,sp_cinit(sp_cabs2(tmp->image->data[i]),0));
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



void ImageItem::setImageCenter(QPointF center){
  if(image){
    addToStack(ImageCenter,center);
    image->detector->image_center[0] = center.x();
    image->detector->image_center[1] = center.y();    
    repositionCenterIndicators();
    _view->emitImageItemChanged(this);
  }
}

QPointF ImageItem::imageCenter() const{
  if(image){
    return QPointF(image->detector->image_center[0],image->detector->image_center[1]);
  }
  return QPointF();
}

QSizeF ImageItem::pixelSize() const{
  if(image){
    return QSizeF(image->detector->pixel_size[0],image->detector->pixel_size[1]);
  }
  return QSizeF();
}

void ImageItem::setPixelSize(QSizeF pixelSize){
  if(image){
    addToStack(PixelSize,pixelSize);
    image->detector->pixel_size[0] = pixelSize.width();
    image->detector->pixel_size[1] = pixelSize.height();
  }
}

void ImageItem::pointConvolute(QPointF scenePos,const Image * kernel){
  if(!image){
    return;
  }
  QVariant img_var;
  qVariantSetValue(img_var,kernel);
  addToStack(PointConvolute,scenePos,img_var);
  QPointF pos = mapFromScene(scenePos);
  int x = (int)pos.x();
  int y = (int)pos.y();
  Complex value = sp_image_get(image,x,y,0);
  long long  index = sp_image_get_index(image,x,y,0);
  value = sp_cscale(value,sp_point_convolute(image,kernel,index)/sp_cabs(value));
  sp_image_set(image,x,y,0,value);
}

void ImageItem::setDetectorDistance(double distance){
  if(image){
    addToStack(DetectorDistance,distance);
    image->detector->detector_distance = distance;
  }
}

double ImageItem::detectorDistance() const{
  if(image){
    return image->detector->detector_distance;
  }
  return -1;
}

void ImageItem::setWavelength(double wavelength){
  if(image){
    addToStack(Wavelength,wavelength);
    image->detector->wavelength = wavelength;
  }
}

double ImageItem::wavelength() const{
  if(image){    
    return image->detector->wavelength;
  }
  return -1;
}

void ImageItem::setShifted(bool shifted){
  if(image){
    addToStack(Shifted,shifted);
    image->shifted = shifted;
  }
}

bool ImageItem::shifted()const{
  if(image){
    return image->shifted;
  }
  return false;
}

void ImageItem::setScaled(bool scaled){
  if(image){
    addToStack(Scaled,scaled);
    image->scaled = scaled;
  }
}

bool ImageItem::scaled()const{
  if(image){
    return image->scaled;
  }
  return false;
}

void ImageItem::setPhased(bool phased){
  if(image){
    addToStack(Phased,phased);
    image->phased = phased;
  }
}

bool ImageItem::phased()const{
  if(image){
    return image->phased;
  }
  return false;
}

void ImageItem::setImageSize(QSize size){
  if(image){
    addToStack(ImageSize,size);
    sp_image_realloc(image,size.width(),size.height(),1);
    updateImage();
  }
}

QSize ImageItem::imageSize() const{
  if(image){
    return QSize(sp_image_x(image),sp_image_y(image));
  }
  return QSize();
}

void ImageItem::addToStack(EditType type, QVariant arg1, QVariant arg2){
  if(undoStack.size() % 10 == 0){
    /* add checkpoints every 10 edits */
    QVariant var;
    Image * copy_img = sp_image_duplicate(image,SP_COPY_ALL);
    qVariantSetValue(var,copy_img);
    QVector<QVariant> args;
    args << var;
    EditStep step = {CheckPoint,args};
    undoStack.push(step);
  }
  if(type == ImageSize || type == Phased || type == Shifted ||
     type == Wavelength || type == DetectorDistance ||
     type == DetectorDistance || type == Scaled || type == PixelSize ||
     type == ImageCenter){
    QVector<QVariant> args;
    args << arg1;
    EditStep step = {type,args};
    undoStack.push(step);
  }else if(type == PointConvolute){
    QVector<QVariant> args;
    args << arg1;
    const Image * a = qVariantValue<const Image *>(arg2);
    if(!a){
      abort();
    }
    QVariant copy_arg;
    Image * copy_img = sp_image_duplicate(a,SP_COPY_ALL);
    qVariantSetValue(copy_arg,copy_img);
    args << copy_arg;
    EditStep step = {type,args};
    undoStack.push(step);
  }
}

void ImageItem::applyEditStep(EditStep step){
  if(step.type == ImageSize){
    setImageSize(step.arguments[0].toSize());
  }
  if(step.type == Phased){
    setPhased(step.arguments[0].toBool());
  }
  if(step.type == Shifted){
    setShifted(step.arguments[0].toBool());
  }
  if(step.type == Wavelength){
    setWavelength(step.arguments[0].toDouble());
  }
  if(step.type == DetectorDistance){
    setDetectorDistance(step.arguments[0].toDouble());
  }
  if(step.type == Scaled){
    setScaled(step.arguments[0].toBool());
  }
  if(step.type == PixelSize){
    setPixelSize(step.arguments[0].toSizeF());
  }
  if(step.type == ImageCenter){
    setImageCenter(step.arguments[0].toPointF());
  }

  if(step.type == CheckPoint){
    sp_image_free(image);
    image = sp_image_duplicate(qVariantValue<Image *>(step.arguments[0]),SP_COPY_ALL);
  }
}

void ImageItem::redoEditSteps(int numSteps){
  if(numSteps > redoStack.size()){
    return;
  }
  for(int i = 0;i<numSteps;i++){
    EditStep step = redoStack.pop();
    applyEditStep(step);
    /* this is not really necessary because the apply will also push to the undo stack */
    /* Checkpoint is the only one that needs to be put again by hand */
    if(step.type == CheckPoint){
      numSteps++;
      undoStack.push(step); 
    }
  }
  updateImage();    
  _view->emitImageItemChanged(this);
}

void ImageItem::undoEditSteps(int numSteps){
  if(numSteps > undoStack.size()){
    return;
  }
  int finalStackSize = undoStack.size()-numSteps;
  while(1){
    redoStack.push(undoStack.pop());
    if(redoStack.top().type == CheckPoint){
      if(undoStack.size() <= finalStackSize){
	break;
      }else{
	/* CheckPoints don't really count as edit steps */	
	finalStackSize--;
      }
    }
    if(undoStack.isEmpty()){
      abort();
    }
  }
  while(undoStack.size() < finalStackSize){
    EditStep step = redoStack.pop();
    applyEditStep(step);
    /* this is not really necessary because the apply will also push to the undo stack */
    /* Checkpoint is the only one that needs to be put again by hand */
    if(step.type == CheckPoint){
        undoStack.push(step); 
    }
  }
  updateImage();    
  _view->emitImageItemChanged(this);
}

void ImageItem::removeVerticalLines(QRect rect){
  if(!getImage()){
    return;
  }
  Image * a = image;
  QVector<double> noise(rect.width());
  noise.fill(0);
  for(int x = 0;x< rect.width();x++){
    for(int y = 0;y< rect.height();y++){
      noise[x] += sp_real(sp_image_get(a,rect.x()+x,rect.y()+y,0));
    }
  }
  for(int x = 0;x< rect.width();x++){
    noise[x] /= rect.height();
  }
  for(int x = 0;x< rect.width();x++){
    for(int y = 0;y<sp_image_y(a);y++){
      Complex v = sp_image_get(a,rect.x()+x,y,0);
      sp_real(v) -= noise[x];
      sp_image_set(a,rect.x()+x,y,0,v);
    }
  }
  updateImage();    
  _view->emitImageItemChanged(this);
}


void ImageItem::removeHorizontalLines(QRect rect){
  if(!getImage()){
    return;
  }
  Image * a = image;
  QVector<double> noise(rect.height());
  noise.fill(0);
  for(int y = 0;y< rect.height();y++){
    for(int x = 0;x< rect.width();x++){
      noise[y] += sp_real(sp_image_get(a,rect.x()+x,rect.y()+y,0));
    }
  }
  for(int y = 0;y< rect.height();y++){
    noise[y] /= rect.width();
  }
  for(int y = 0;y<rect.height();y++){
    for(int x = 0;x< sp_image_x(a);x++){
      Complex v = sp_image_get(a,x,rect.y()+y,0);
      sp_real(v) -= noise[y];
      sp_image_set(a,x,rect.y()+y,0,v);
    }
  }
  updateImage();    
  _view->emitImageItemChanged(this);
}


void ImageItem::repositionCenterIndicators(){
  centerVerticalIndicator->setLine(imageCenter().x(),-100000,imageCenter().x(),100000);
  centerHorizontalIndicator->setLine(-100000,imageCenter().y(),100000,imageCenter().y());
}

void ImageItem::setCenterIndicatorsVisible(bool show){
  centerVerticalIndicator->setVisible(show);
  centerHorizontalIndicator->setVisible(show);
}

void ImageItem::setSelected(bool selected){
  selectRect->setVisible(selected);
}

void ImageItem::rotateImage(){
  sp_image_reflect(image,1,SP_AXIS_XY);
  updateImage();
}

void ImageItem::xcamPreprocess(){
  Image * a = xcam_preprocess(image);
  sp_image_free(image);
  image = a;  
  updateImage();
}

void ImageItem::interpolateEmpty(double radius,int iterations, QRegion selected){
  if(!image || radius <= 0 || iterations < 1){
    return;
  }
  if(selected.isEmpty()){
    qDebug("No selection radius %f",radius);
    selected = QRect(0,0,sp_image_x(image),sp_image_y(image));
  }else{
    qDebug("Selection radius %f",radius);
  }
  for(int i = 0;i<iterations;i++){
    Image * after = gaussian_blur(image,radius);
    for(int x = 0;x<sp_image_x(image);x++){
      for(int y = 0;y<sp_image_y(image);y++){
	if(sp_image_mask_get(image,x,y,0) == 0 && selected.contains(QPoint(x,y))){
	  sp_image_set(image,x,y,0,sp_image_get(after,x,y,0));
	}
      }
    }
    sp_image_free(after);
  }
  updateImage();
}

void ImageItem::cropImage(QRegion selected){
  if(selected.isEmpty() || !image){
    return;
  }
  QRect r = selected.boundingRect();
  Image * tmp = rectangle_crop(image,r.x(),r.y(),r.x()+r.width(),r.y()+r.height());
  sp_image_free(image);
  image = tmp;
  updateImage();
}


void ImageItem::showIdentifier(bool show){
  if(!identifierItem){
    identifierItem = new QGraphicsTextItem(identifierString);
    identifierItem->setDefaultTextColor(Qt::white);
    qreal posy = -(identifierItem->boundingRect()).height();
    identifierItem->setPos(boundingRect().width()/2-(identifierItem->boundingRect()).width()/2,posy);
  }
  if(show){
    identifierItem->show();
  }else{
    identifierItem->hide();
  }
}

QString ImageItem::identifier() const{
  return identifierString;
}

double ImageItem::dx() const{
  return pos().x();
}

void ImageItem::setDx(double new_dx){
  setPos(new_dx,pos().y());
}

double ImageItem::dy() const{
  return pos().y();
}

void ImageItem::setDy(double new_dy){
  setPos(pos().x(),new_dy);
}

double ImageItem::dz() const{
  const double  defaultDistance = 50.0;
  return defaultDistance/overallScale();
}

void ImageItem::setDz(double new_dz){
  const double  defaultDistance = 50.0;
  double new_scale = defaultDistance/new_dz;

  /* Important to keep in mind that translate is *NOT* the same as changing dx and dy directly
   but is affected by m11() and m22() */

  /* remove translation */
  setTransform(transform().translate(pixmap().width()/2,pixmap().height()/2));
  
  /* do the scaling */
  double toScale = new_scale/overallScale();
  scale(toScale,toScale);

  /* add translation. This is not useless! */
  setTransform(transform().translate(-pixmap().width()/2,-pixmap().height()/2));
}


void ImageItem::setTheta(double new_theta){
  double delta_theta = (new_theta-theta());
  /* remove translation */
  setTransform(transform().translate(pixmap().width()/2,pixmap().height()/2));

  rotate(-delta_theta);

  /* add translation. This is not useless! */
  setTransform(transform().translate(-pixmap().width()/2,-pixmap().height()/2));

}

double ImageItem::theta() const{
  QPointF a = transform().map(QPointF(pixmap().width()/2+1.0,pixmap().height()/2));
  /* we have to negate y because in 2D graphics positive y points down */
  return atan2(-a.y(),a.x())*180.0/M_PI;
}

double ImageItem::overallScale() const{
  return sqrt(transform().determinant());
  //  return sqrt(transform().m11()*transform().m22());
}

void ImageItem::addControlPoint(QPointF pos){
  QGraphicsEllipseItem * point = new QGraphicsEllipseItem(-0.2,-0.2,0.4,0.4,this);
  point->setPos(pos);
  point->setBrush(Qt::white);
  QPen p = point->pen();
  p.setCosmetic(true);
  p.setWidth(0);
  p.setColor(Qt::white);
  point->setPen(p);
  controlPoints.append(point);
  QGraphicsTextItem * label = new QGraphicsTextItem(QString::number(controlPoints.size()),point);
  label->setDefaultTextColor(Qt::white);
  label->setPos(-(label->boundingRect()).width()/2,-(label->boundingRect()).height()*0.8);
  //  scene()->addItem(point);
  qDebug("Add point at %f %f",pos.x(),pos.y());
}

void ImageItem::deleteControlPoint(QPointF pos){
  /* 10 px tolerance radius, delete the closest */
  double tolerance2 = 10*10;
  int toDelete = -1;
  double toDeleteDistance2 = tolerance2;
  for(int i = 0;i<controlPoints.size();i++){
    if(controlPoints[i]){
      QPointF d = controlPoints[i]->pos()-pos;
      if(d.x()*d.x()+d.y()*d.y() < tolerance2 && d.x()*d.x()+d.y()*d.y() < toDeleteDistance2){
	toDelete = i;
	toDeleteDistance2 = d.x()*d.x()+d.y()*d.y();      
      }
    }
  }
  if(toDelete >= 0){
    delete controlPoints[toDelete];
    controlPoints[toDelete] = NULL;
  }
}
