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
  identifierItem->translate(-identifierItem->boundingRect().width()/2,posy);
  identifierItem->setPos(boundingRect().width()/2,0);
  identifierItem->setFlags(identifierItem->flags() | QGraphicsItem::ItemIgnoresTransformations);
  identifierItem->setZValue(zValue()+1);
  _dxLocked = false;
  _dyLocked = false;
  _dzLocked = false;
  _thetaLocked = false;
  _alphaLocked = false;
  _alpha = 0;
  _theta = 0;
  _dx = 0;
  _dy = 0;
  _dz = 1;
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
  QRectF sr = mapRectFromScene(QRectF(area.topLeft(),area.bottomRight())).normalized();
  QPointF tl = sr.topLeft(); 
  QPointF br = sr.bottomRight();
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
  if(centerVerticalIndicator){
    centerVerticalIndicator->setVisible(show);
  }
  if(centerHorizontalIndicator){
    centerHorizontalIndicator->setVisible(show);
  }
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

void ImageItem::interpolateEmpty(double radius,int iterations, QRegion selected,QString kernel_type){
  if(!image || radius <= 0 || iterations < 1){
    return;
  }
  if(selected.isEmpty()){
    qDebug("No selection radius %f",radius);
    selected = QRect(0,0,sp_image_x(image),sp_image_y(image));
  }else{
    qDebug("Selection radius %f",radius);
  }
  Image * kernel = sp_image_alloc(sp_image_x(image),sp_image_y(image),1);
  sp_image_fill(kernel,sp_cinit(0,0));
  if(kernel_type == "Sinc"){
    for(int x = 0;x<sp_image_x(image);x++){
      for(int y = 0;y<sp_image_y(image);y++){
	if(fabs(x-image->detector->image_center[0]) < sp_image_x(image)/(2*radius) &&
	   fabs(y-image->detector->image_center[1]) < sp_image_y(image)/(2*radius)){
	  sp_image_set(kernel,x,y,0,sp_cinit(1,0));
	}
      }
    }  
    //    sp_image_scale(kernel,sqrt(sp_image_size(kernel)/sp_cabs(sp_image_integrate(kernel))));
    //    sp_image_scale(kernel,1.0/sp_cabs(sp_image_integrate(kernel)));
    Image * kernel_shift = sp_image_shift(kernel);
    sp_image_free(kernel);
    kernel = kernel_shift;
  }else{
    for(int i = 0;i<sp_image_size(image);i++){
      real dist = sp_image_dist(image,i,SP_TO_CENTER);
      real v = exp(-dist*dist/(2*radius*radius));
      sp_image_set_by_index(kernel,i,sp_cinit(v,0));
    }
    sp_image_scale(kernel,1.0/sp_cabs(sp_image_integrate(kernel)));
    Image * kernel_shift = sp_image_shift(kernel);
    sp_image_free(kernel);
    kernel = sp_image_fft(kernel_shift);
    //    sp_image_scale(kernel,1.0/sqrt(sp_image_size(kernel)));
    sp_image_free(kernel_shift);  
  }
  for(int i = 0;i<iterations;i++){
    Image * a = sp_image_fft(image);
    sp_image_mul_elements(a,kernel);
    Image * after = sp_image_ifft(a);
    sp_image_scale(after,1.0/sp_image_size(after));
    sp_image_free(a);
    //      Image * after = gaussian_blur(image,radius);
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
  return _dx;
}

void ImageItem::setDx(double new_dx){
  if(_dxLocked == true){
    return;
  }
  _dx = new_dx;
  setTransform(transformFromParameters());
}

double ImageItem::dy() const{
  return _dy;
}

void ImageItem::setDy(double new_dy){
  if(_dyLocked == true){
    return;
  }
  _dy = new_dy;
  setTransform(transformFromParameters());
}

double ImageItem::dz() const{
  return _dz;
}

void ImageItem::setDz(double new_dz){
  if(_dzLocked == true){
    return;
  }
  _dz = new_dz;
  setZValue(1.0/dz());
  setTransform(transformFromParameters());

}

void ImageItem::setTheta(double new_theta){
  if(_thetaLocked){
    return;
  }
  _theta = new_theta;

  setTransform(transformFromParameters());
}

double ImageItem::theta() const{
  return _theta;
}


void ImageItem::setAlpha(double new_alpha){
  if(_alphaLocked){
    return;
  }
  _alpha = new_alpha;
  setTransform(transformFromParameters());
}

double ImageItem::alpha() const{
  return _alpha;
}


bool ImageItem::dxLocked() const{
  return _dxLocked;
}

void ImageItem::setDxLocked(bool locked){
  _dxLocked = locked;
}

bool ImageItem::dyLocked() const{
  return _dyLocked;
}

void ImageItem::setDyLocked(bool locked){
  _dyLocked = locked;
}

bool ImageItem::dzLocked() const{
  return _dzLocked;
}

void ImageItem::setDzLocked(bool locked){
  _dzLocked = locked;
}

bool ImageItem::thetaLocked() const{
  return _thetaLocked;
}

void ImageItem::setThetaLocked(bool locked){
  _thetaLocked = locked;
}


bool ImageItem::alphaLocked() const{
  return _alphaLocked;
}

void ImageItem::setAlphaLocked(bool locked){
  _alphaLocked = locked;
}


double ImageItem::overallScale() const{
  return sqrt(transform().determinant());
  //  return sqrt(transform().m11()*transform().m22());
}

void ImageItem::addControlPoint(QPointF pos){
  QGraphicsEllipseItem * point = new QGraphicsEllipseItem(-1,-1,2,2,this);
  point->setPos(pos);
  point->setBrush(Qt::white);
  QPen p = point->pen();
  p.setCosmetic(true);
  p.setWidth(0);
  p.setColor(Qt::white);
  point->setPen(p);
  point->setFlags(point->flags() | QGraphicsItem::ItemIgnoresTransformations);
  point->setZValue(zValue()+1);
  controlPoints.append(point);
  QGraphicsTextItem * label = new QGraphicsTextItem(QString::number(controlPoints.size()),point);
  label->setDefaultTextColor(Qt::white);
  //  label->translate(-(label->boundingRect()).width()/2,-(label->boundingRect()).height()*0.8);
  label->setFlags(label->flags() | QGraphicsItem::ItemIgnoresTransformations);
  label->setZValue(zValue()+1);
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

QList<QPointF> ImageItem::getControlPoints(){
  QList<QPointF> ret;
  for(int i = 0;i<controlPoints.size();i++){
    ret.append(controlPoints[i]->pos());
  }
  return ret;
}


void ImageItem::moveBy(qreal dx, qreal dy){
  if(dxLocked()){
    dx = 0;
  }
  if(dyLocked()){
    dy = 0;
  }
  QGraphicsItem::moveBy(dx,dy);
}


QTransform ImageItem::transformFromParameters(){
  QTransform t;
  t.rotateRadians(theta(),Qt::ZAxis);
  t.rotateRadians(alpha(),Qt::YAxis);
  t.scale(1.0/dz(),1.0/dz());
  t.translate(dx(),dy());
  /* add translation. This is not useless! */
  //  t = QTransform(t.m11(), t.m12(), t.m13()+dz()/1024, t.m21(), t.m22(), t.m23()+dz()/1024, t.m31(), t.m32(),1);
  t.translate(-pixmap().width()/2,-pixmap().height()/2);  
  return t;
}


void ImageItem::setImageMask(QPoint pos,int value){
  if(!image){
    return;
  }
  addToStack(ImageMask,pos,value);
  int x = pos.x();
  int y = pos.y();
  if(sp_image_contains_coordinates(image,x,y,0)){
    sp_image_mask_set(image,x,y,0,value);
  }
}


QList<QPoint> ImageItem::imagePointsAround(QPointF scenePos,int sceneRadius){
  QList<QPoint> ret;
  if(!image){
    return ret;
  }
  QPoint center(mapFromScene(scenePos).x(),mapFromScene(scenePos).y());
  QPoint topLeft(mapFromScene(scenePos-QPointF(sceneRadius,sceneRadius)).x(),
		    mapFromScene(scenePos-QPointF(sceneRadius,sceneRadius)).y());
  QPoint bottomRight(mapFromScene(scenePos+QPointF(sceneRadius,sceneRadius)).x(),
		     mapFromScene(scenePos+QPointF(sceneRadius,sceneRadius)).y());
  QPointF tmp = mapFromScene(scenePos)-mapFromScene(scenePos+QPointF(sceneRadius,0));
  real radius = sqrt(tmp.x()*tmp.x()+tmp.y()*tmp.y());
  for(int x = topLeft.x();x<=bottomRight.x()+1;x++){
    for(int y = topLeft.y();y<=bottomRight.y()+1;y++){
      if(!sp_image_contains_coordinates(image,x,y,0)){
	continue;
      }
      QPoint aroundPos(x,y);      
      QPointF diff = center-aroundPos;
      if(diff.x()*diff.x() + diff.y()*diff.y() > radius*radius){
	continue;
      }
      ret.append(aroundPos);
    }
  }
  return ret;
}

void ImageItem::setMaskFromImage(const Image * mask){
  if(!image){
    return;
  }
  QVariant img_var;
  qVariantSetValue(img_var,mask);
  addToStack(MaskFromImage,img_var);
  for(int i = 0;i<sp_image_size(mask);i++){
    if(sp_real(mask->image->data[i]) == 0){
      image->mask->data[i] = 0;
    }else{
      image->mask->data[i] = 1;
    }
  }
}

void ImageItem::invertMask(){
  if(!image){
    return;
  }
  addToStack(InvertMask);
  for(int i = 0;i<sp_image_size(image);i++){
    if(image->mask->data[i] == 0){
      image->mask->data[i] = 1;
    }else{
      image->mask->data[i] = 0;
    }
  }
}
