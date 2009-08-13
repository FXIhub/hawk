#include "imageeditorview.h"
#include "imageitem.h"
#include "lineoutplot.h"
#include <QtGui>

ImageEditorView::ImageEditorView(QWidget * parent)
  :ImageView(parent)
{
  setPreserveShift(false);
  mode = EditorDefaultMode;
  dropBrushRadius = 3.0;
  dropBlurRadius = 3.0;
  generateDropCursor();
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  rubberBand->hide();
  selectedRegion = QRegion();

}

void ImageEditorView::setImageCenter(QPointF center){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->detector->image_center[0] = center.x();
    a->detector->image_center[1] = center.y();    
  }
}

QPointF ImageEditorView::imageCenter() const{
  QPointF ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    ret = QPointF(a->detector->image_center[0],a->detector->image_center[1]);
  }
  return ret;
}

QSize ImageEditorView::pixelSize() const{
  QSize ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    ret = QSize(a->detector->pixel_size[0],a->detector->pixel_size[1]);
  }
  return ret;
}

void ImageEditorView::setPixelSize(QSize pixelSize){
  QSize ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->detector->pixel_size[0] = pixelSize.width();
    a->detector->pixel_size[1] = pixelSize.height();
  }
}

QSize ImageEditorView::imageSize() const{
  QSize ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    ret = QSize(sp_image_x(a),sp_image_y(a));
  }
  return ret;
}

void ImageEditorView::setImageSize(QSize imageSize){
  QSize ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    imageItem()->reallocImage(imageSize);
  }
}


bool ImageEditorView::phased() const{
  bool phased = false;
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    phased = a->phased;
  }
  return phased;
}

bool ImageEditorView::scaled() const{
  bool scaled = false;
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    scaled = a->scaled;
  }
  return scaled;
}

void ImageEditorView::setPhased(bool phased){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->phased = phased;
  }
}

void ImageEditorView::setScaled(bool scaled){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->scaled = scaled;
  }
}

bool ImageEditorView::shifted() const{
  bool shifted = false;
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    shifted = a->shifted;
  }
  return shifted;
}


void ImageEditorView::setShifted(bool shifted){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->shifted = shifted;
  }
} 

void ImageEditorView::setWavelength(double wavelength){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->detector->wavelength = wavelength;
  }
} 

double ImageEditorView::wavelength() const{
  double wavelength = -1;
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    wavelength = a->detector->wavelength;
  }
  return wavelength;
}


double ImageEditorView::detectorDistance() const{
  double detectorDistance = -1;
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    detectorDistance = a->detector->detector_distance;
  }
  return detectorDistance;
}

void ImageEditorView::setDetectorDistance(double detectorDistance){
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    a->detector->detector_distance = detectorDistance;
  }
} 

QString ImageEditorView::propertyNameToDisplayName(QString propertyName){
  /* first remove the HawkImage tag */
  QString tag = "HawkImage_";
  if(!propertyName.startsWith(tag)){
    return QString();
  }
  propertyName.remove(0,tag.length());
  propertyName[0] = propertyName[0].toUpper();
  for(int i = 1;i<propertyName.length();i++){
    QChar c = propertyName[i];
    if(c.isUpper()){
      /* insert space before upper case letters */
      propertyName.insert(i," ");
      i++;
    }    
  }
  return propertyName;
}

void ImageEditorView::mouseReleaseEvent( QMouseEvent *  event){
  if(mode == EditorDefaultMode || (event->modifiers() & Qt::ShiftModifier)){
    ImageView::mouseReleaseEvent(event);
  }else if(mode == EditorBlurMode){
    /* Blur the area around the press */
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it.at(i));
      if(ii){
	Image * image = ii->getImage();
	if(image){
	  Image * kernel = getBlurKernel();
	  for(real xi = event->pos().x()-getDropBrushRadius();xi<=event->pos().x()+getDropBrushRadius();xi++){
	    for(real yi = event->pos().y()-getDropBrushRadius();yi<=event->pos().y()+getDropBrushRadius();yi++){
	      QPoint aroundPos(xi,yi);
	      QPointF diff = event->pos()-aroundPos;
	      if(diff.x()*diff.x() + diff.y()*diff.y() > getDropBrushRadius()*getDropBrushRadius()){
		continue;
	      }
	      applyDrop(it.at(i)->mapFromScene(mapToScene(aroundPos)),image,kernel);
	    }
	  }
	  sp_image_free(kernel);
	  imageItem()->updateImage();
	  emit imageItemChanged(imageItem());
	}
      }
    }
  }else if(mode == EditorSelectionMode){
    if(1){
      /*select region in image coordinates */
      if(imageItem() && imageItem()->getImage()){
	selectedRegion += QRect(0,0,sp_image_x(imageItem()->getImage()),sp_image_y(imageItem()->getImage())).intersected(QRect(imageItem()->mapFromScene(mapToScene(rubberBandOrigin)).toPoint(),imageItem()->mapFromScene(mapToScene(event->pos())).toPoint()).normalized());
	scene()->update();
      }
    }
    rubberBand->hide();
  }else if(mode == EditorLineoutMode){
    if(imageItem() && imageItem()->getImage()){ 
      QLineF line = QLineF(imageItem()->mapFromScene(mapToScene(lineOutOrigin)),imageItem()->mapFromScene(mapToScene(lineOutEnd)));
      new LineOutPlot(imageItem()->getImage(),line);
    }
    scene()->update();
  }
}

void ImageEditorView::mouseMoveEvent(QMouseEvent *event){
  if(mode == EditorDefaultMode || (event->modifiers() & Qt::ShiftModifier)){
    ImageView::mouseMoveEvent(event);
    return;
  }else if(mode == EditorBlurMode && (event->buttons() & Qt::LeftButton)){
    /* Blur the area around the press */
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it.at(i));
      if(ii){
	Image * image = ii->getImage();
	if(image){
	  Image * kernel = getBlurKernel();
	  for(real xi = event->pos().x()-getDropBrushRadius();xi<=event->pos().x()+getDropBrushRadius();xi++){
	    for(real yi = event->pos().y()-getDropBrushRadius();yi<=event->pos().y()+getDropBrushRadius();yi++){
	      QPoint aroundPos(xi,yi);
	      QPointF diff = event->pos()-aroundPos;
	      if(diff.x()*diff.x() + diff.y()*diff.y() > getDropBrushRadius()*getDropBrushRadius()){
		continue;
	      }
	      applyDrop(it.at(i)->mapFromScene(mapToScene(aroundPos)),image,kernel);
	    }
	  }
	  sp_image_free(kernel);
	  /* just update on the mouse release */
	  /*
	    imageItem()->updateImage();
	    emit imageItemChanged(imageItem());
	  */
	}
      }
    }
  }else if(mode == EditorSelectionMode){
    rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
  }else if(mode == EditorLineoutMode){
    lineOutEnd = event->pos();
    scene()->update();
  }
  mouseOverValue(event);
}

void ImageEditorView::mousePressEvent(QMouseEvent *event){
  if(mode == EditorDefaultMode || (event->modifiers() & Qt::ShiftModifier)){
    ImageView::mousePressEvent(event);
    return;
  }else if(mode == EditorSelectionMode){
    rubberBandOrigin = event->pos();
    rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
    rubberBand->show();
  }else if(mode == EditorLineoutMode){
    lineOutOrigin = event->pos();
  }
}

void ImageEditorView::setBlurMode(){
  mode = EditorBlurMode;
  setCursor(QCursor(dropCursor));
}

void ImageEditorView::setSelectionMode(){
  mode = EditorSelectionMode;
  setCursor(QCursor(Qt::CrossCursor));
}

void ImageEditorView::setLineoutMode(){
  mode = EditorLineoutMode;
  setCursor(QCursor(Qt::CrossCursor));
}

void ImageEditorView::setDefaultMode(){
  mode = EditorDefaultMode;
  setCursor(QCursor(Qt::ArrowCursor));
}

EditorMode ImageEditorView::editorMode(){
  return mode;
}

void ImageEditorView::wheelEvent( QWheelEvent * event){
  if(mode == EditorDefaultMode | (event->modifiers() & Qt::ShiftModifier)){
    ImageView::wheelEvent(event);
  }
}


Image * ImageEditorView::getBlurKernel(){
  Image * kernel = sp_image_alloc(2*getDropBlurRadius()+1,2*getDropBlurRadius()+1,1);
  /* we're gonna go for the simple flat kernel for now */
  kernel->detector->image_center[0] = getDropBlurRadius();
  kernel->detector->image_center[1] = getDropBlurRadius();
  /* this z center is important! */
  kernel->detector->image_center[2] = 0;
  for(int i = 0;i<sp_image_size(kernel);i++){
    if(sp_image_dist(kernel,i,SP_TO_CENTER) <= getDropBlurRadius()){
      sp_real(kernel->image->data[i]) = 1.0;
      sp_imag(kernel->image->data[i]) = 0;
    }
  }
  /* normalize */
  sp_image_scale(kernel,1.0/sp_image_integrate2(kernel));
  return kernel;
}

void ImageEditorView::generateDropCursor(){
  dropCursor = QPixmap((dropBrushRadius+dropBlurRadius)*2+2,(dropBrushRadius+dropBlurRadius)*2+2);
  QPainter painter(&dropCursor);
  painter.setRenderHints(QPainter::Antialiasing);
  dropCursor.fill(Qt::transparent);
  painter.drawEllipse(QRect(1+dropBlurRadius,1+dropBlurRadius,(dropBrushRadius)*2,(dropBrushRadius)*2));
  painter.setPen(Qt::DashLine);
  painter.drawEllipse(QRect(1,1,(dropBrushRadius+dropBlurRadius)*2,(dropBrushRadius+dropBlurRadius)*2));
  if(editorMode() == EditorBlurMode){
    setCursor(QCursor(dropCursor));
  }
}

void ImageEditorView::setDropBrushRadius(double d){
  dropBrushRadius = d;
  generateDropCursor();
}

void ImageEditorView::setDropBlurRadius(double d){
  dropBlurRadius = d;
  generateDropCursor();
}

double ImageEditorView::getDropBrushRadius(){
  return dropBrushRadius;
}

double ImageEditorView::getDropBlurRadius(){
  return dropBlurRadius;
}

void ImageEditorView::applyDrop(QPointF pos,Image * image, Image * kernel){
  int x = (int)pos.x();
  if(x < 0){
    x = 0;
  }
  if(x >= sp_image_x(image)){
    x = sp_image_x(image)-1;
  }
	  
  int y = (int)pos.y();
  if(y < 0){
    y = 0;
  }
  if(y >= sp_image_y(image)){
    y = sp_image_y(image)-1;
  }
  for(int xi = x-getDropBrushRadius();xi<=x-getDropBrushRadius();xi++){
    if(xi < 0 || xi >= sp_image_x(image)){
      continue;
    }
    for(int yi = y-getDropBrushRadius();yi<=y-getDropBrushRadius();yi++){
      if(yi < 0 || yi >= sp_image_y(image)){
	continue;
      }
      long long index = sp_image_get_index(image,x,y,0);      
      /* only change the magnitude */
      Complex value = sp_image_get(image,x,y,0);
      value = sp_cscale(value,sp_point_convolute(image,kernel,index)/sp_cabs(value));
      sp_image_set(image,x,y,0,value);
      qDebug("Doing convolute at %d %d",x,y);
    }
  }
}


void ImageEditorView::paintEvent(QPaintEvent * event ){
  ImageView::paintEvent(event);
  /* draw selected regions */
  QPainter p(viewport());
  p.setRenderHint(QPainter::Antialiasing);
  qDebug("paint Event");
  if(!selectedRegion.isEmpty() && imageItem()){
    QPainterPath path;
    path.addRegion(selectedRegion);
    QBrush brush = p.brush();
    brush.setColor(QColor(0,0,0,50));
    brush.setStyle(Qt::SolidPattern);
    p.setBrush(brush);
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setWidthF(1.5);
    pen.setStyle(Qt::DotLine);
    p.setPen(pen);
    p.drawPath(mapFromScene(imageItem()->mapToScene(path.simplified())));
  }
  if(mode == EditorLineoutMode && QApplication::mouseButtons() & Qt::LeftButton){
    /* paint line out */
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawLine(lineOutOrigin,lineOutEnd);
  }  
}
