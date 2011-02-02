#include "imageeditorview.h"
#include "imageitem.h"
#include "lineoutplot.h"
#include "editorworkspace.h"
#include "editortools.h"
#include <QtGui>

ImageEditorView::ImageEditorView(QWidget * parent,EditorWorkspace * workspace)
  :ImageView(parent)
{
  editorWorkspace = workspace;
  setPreserveShift(false);
  mode = EditorDefaultMode;
  dropBrushRadius = 3.0;
  dropBlurRadius = 3.0;
  generateDropCursor();
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  rubberBand->hide();
  _selectedRegion = QRegion();

}

void ImageEditorView::setImageCenter(QPointF center){
  if(selectedImage()){
    selectedImage()->setImageCenter(center);
  }
}

QPointF ImageEditorView::imageCenter() const{
  if(selectedImage()){
    return selectedImage()->imageCenter();
  }
  return QPointF();
}

QSizeF ImageEditorView::pixelSize() const{
  QSizeF ret;
  if(selectedImage()){
    ret = selectedImage()->pixelSize();
  }
  return ret;
}

void ImageEditorView::setPixelSize(QSizeF pixelSize){
  if(selectedImage()){
    selectedImage()->setPixelSize(pixelSize);
  }
}

QSize ImageEditorView::imageSize() const{
  QSize ret(0,0);
  if(selectedImage()){
    return selectedImage()->imageSize();
  }
  return ret;
}

void ImageEditorView::setImageSize(QSize imageSize){
  if(selectedImage()){
    selectedImage()->setImageSize(imageSize);
  }
}


bool ImageEditorView::phased() const{
  if(selectedImage()){
    return selectedImage()->phased();
  }
  return false;
}

bool ImageEditorView::scaled() const{
  if(selectedImage()){
    return selectedImage()->scaled();
  }
  return false;
}

void ImageEditorView::setPhased(bool phased){
  if(selectedImage()){
    selectedImage()->setPhased(phased);
  }
}

void ImageEditorView::setScaled(bool scaled){
  if(selectedImage()){
    selectedImage()->setScaled(scaled);
  }
}

bool ImageEditorView::shifted() const{
  if(selectedImage()){
    return selectedImage()->shifted();
  }
  return false;
}


void ImageEditorView::setShifted(bool shifted){
  if(selectedImage()){
    selectedImage()->setShifted(shifted);
  }
} 

void ImageEditorView::setWavelength(double wavelength){
  if(selectedImage()){
    selectedImage()->setWavelength(wavelength);
  }
} 

double ImageEditorView::wavelength() const{
  if(selectedImage()){
    return selectedImage()->wavelength();
  }
  return -1;
}


double ImageEditorView::detectorDistance() const{
  if(selectedImage()){
    return selectedImage()->detectorDistance();
  }
  return -1;
}

void ImageEditorView::setDetectorDistance(double detectorDistance){
  if(selectedImage()){
    selectedImage()->setDetectorDistance(detectorDistance);
  }
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
	const Image * image = ii->getImage();
	if(image){
	  Image * kernel = getBlurKernel();
	  for(real xi = event->pos().x()-getDropBrushRadius();xi<=event->pos().x()+getDropBrushRadius();xi++){
	    for(real yi = event->pos().y()-getDropBrushRadius();yi<=event->pos().y()+getDropBrushRadius();yi++){
	      QPoint aroundPos(xi,yi);
	      QPointF diff = event->pos()-aroundPos;
	      if(diff.x()*diff.x() + diff.y()*diff.y() > getDropBrushRadius()*getDropBrushRadius()){
		continue;
	      }
	      ii->pointConvolute(mapToScene(aroundPos),kernel);
	    }
	  }
	  sp_image_free(kernel);
	  selectedImage()->updateImage();
	  emit imageItemChanged(selectedImage());
	}
      }
    }
  }else if(mode == EditorSelectionMode){
    EditorTools * tools = editorTools();
    if(tools->selectionMode() == EditorTools::SelectionUnite){
      /*select region in image coordinates */
      if(selectedImage() && selectedImage()->getImage()){
	_selectedRegion += QRect(0,0,sp_image_x(selectedImage()->getImage()),sp_image_y(selectedImage()->getImage())).intersected(QRect(selectedImage()->mapFromScene(mapToScene(rubberBandOrigin)).toPoint(),selectedImage()->mapFromScene(mapToScene(event->pos())).toPoint()).normalized());
	scene()->update();
      }
    }else if(tools->selectionMode() == EditorTools::SelectionSet){
      /*select region in image coordinates */
      if(selectedImage() && selectedImage()->getImage()){
	QRect newRect = QRect(0,0,sp_image_x(selectedImage()->getImage()),sp_image_y(selectedImage()->getImage())).intersected(QRect(selectedImage()->mapFromScene(mapToScene(rubberBandOrigin)).toPoint(),selectedImage()->mapFromScene(mapToScene(event->pos())).toPoint()).normalized());
	if(newRect.width() > 1 || newRect.height() > 1) {
	  _selectedRegion = newRect;
	}else{
	  _selectedRegion = QRect();
	}
	scene()->update();
      }
    }else if(tools->selectionMode() == EditorTools::SelectionSubtract){
      /*select region in image coordinates */
      if(selectedImage() && selectedImage()->getImage()){
	_selectedRegion -= QRect(0,0,sp_image_x(selectedImage()->getImage()),sp_image_y(selectedImage()->getImage())).intersected(QRect(selectedImage()->mapFromScene(mapToScene(rubberBandOrigin)).toPoint(),selectedImage()->mapFromScene(mapToScene(event->pos())).toPoint()).normalized());
	scene()->update();
      }
    }
    rubberBand->hide();
  }else if(mode == EditorLineoutMode){
    if(selectedImage() && selectedImage()->getImage()){ 
      QLineF line = QLineF(selectedImage()->mapFromScene(mapToScene(lineOutOrigin)),selectedImage()->mapFromScene(mapToScene(lineOutEnd)));
      new LineOutPlot(selectedImage()->getImage(),line);
    }
    scene()->update();
  }else if(mode == EditorEditMaskMode){
    editMaskAt(event->pos());
  }
}

void ImageEditorView::mouseMoveEvent(QMouseEvent *event){
  if(mode == EditorDefaultMode || (event->modifiers() & Qt::ShiftModifier)){
    ImageView::mouseMoveEvent(event);
    if(mode == EditorBullseyeMode){
      /* we need to redraw the two lines */
      scene()->update();
    }
    return;
  }else if(mode == EditorBlurMode && (event->buttons() & Qt::LeftButton)){
    /* Blur the area around the press */
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it.at(i));
      if(ii){
	const Image * image = ii->getImage();
	if(image){
	  Image * kernel = getBlurKernel();
	  for(real xi = event->pos().x()-getDropBrushRadius();xi<=event->pos().x()+getDropBrushRadius();xi++){
	    for(real yi = event->pos().y()-getDropBrushRadius();yi<=event->pos().y()+getDropBrushRadius();yi++){
	      QPoint aroundPos(xi,yi);
	      QPointF diff = event->pos()-aroundPos;
	      if(diff.x()*diff.x() + diff.y()*diff.y() > getDropBrushRadius()*getDropBrushRadius()){
		continue;
	      }
	      ii->pointConvolute(mapToScene(aroundPos),kernel);
	    }
	  }
	  sp_image_free(kernel);
	  /* just update on the mouse release */
	  /*
	    selectedImage()->updateImage();
	    emit imageItemChanged(selectedImage());
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
  }else if(mode == EditorBullseyeMode){
    if(selectedImage()){
      selectedImage()->setImageCenter(selectedImage()->mapFromScene(mapToScene(event->pos())));
      scene()->update();
    }
  }
}

void ImageEditorView::setBlurMode(){
  mode = EditorBlurMode;
  setCursor(QCursor(dropCursor));
}

void ImageEditorView::setBullseyeMode(bool on){
  if(on){
    mode = EditorBullseyeMode;
    setCursor(QCursor(Qt::CrossCursor));
    if(selectedImage()){
      selectedImage()->setCenterIndicatorsVisible(true);
    }
  }else{
    if(selectedImage()){
      selectedImage()->setCenterIndicatorsVisible(false);
    }
  }
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

ImageEditorView::EditorMode ImageEditorView::editorMode(){
  return mode;
}

void ImageEditorView::wheelEvent( QWheelEvent * event){
  if(mode == EditorDefaultMode || (event->modifiers() & Qt::ShiftModifier)){
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
      sp_image_set_by_index(kernel,i,sp_cinit(1,0));
    }
  }
  /* normalize */
  sp_image_scale(kernel,1.0/sp_image_integrate2(kernel));
  return kernel;
}

void ImageEditorView::generateDropCursor(){
  dropCursor = QPixmap((dropBrushRadius+dropBlurRadius)*2+2,(dropBrushRadius+dropBlurRadius)*2+2);
  dropCursor.fill(Qt::transparent);

  QPainter painter(&dropCursor);
  painter.setRenderHints(QPainter::Antialiasing);
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


void ImageEditorView::paintEvent(QPaintEvent * event ){
  ImageView::paintEvent(event);
  /* draw selected regions */
  QPainter p(viewport());
  p.setRenderHint(QPainter::Antialiasing);
  if(!selectedRegion().isEmpty() && selectedImage()){
    QPainterPath path;
    path.addRegion(selectedRegion());
    QBrush brush = p.brush();
    brush.setColor(QColor(0,0,0,50));
    brush.setStyle(Qt::SolidPattern);
    p.setBrush(brush);
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setWidthF(1.5);
    pen.setStyle(Qt::DotLine);
    p.setPen(pen);
    p.drawPath(mapFromScene(selectedImage()->mapToScene(path.simplified())));
  }
  if(mode == EditorLineoutMode && QApplication::mouseButtons() & Qt::LeftButton){
    /* paint line out */
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawLine(lineOutOrigin,lineOutEnd);
  }  
    if(mode == EditorBullseyeMode && 0){
    if(selectedImage() && !selectedImage()->imageCenter().isNull()){
      QPointF center = mapFromScene(selectedImage()->mapToScene(selectedImage()->imageCenter()));
      /* paint line out */
      QPen pen = p.pen();
      pen.setColor(Qt::white);
      pen.setStyle(Qt::SolidLine);
      p.setPen(pen);
      p.drawLine(center.x(),0,center.x(),height());
      p.drawLine(0,center.y(),width(),center.y());
      //      scene()->update();
    }
  }  
}

QRegion ImageEditorView::selectedRegion(){
  return _selectedRegion;
}

void ImageEditorView::selectRegion(QRegion region){
  EditorTools * tools = editorTools();
  if(tools->selectionMode() == EditorTools::SelectionUnite){
    _selectedRegion += region;
  }
  if(tools->selectionMode() == EditorTools::SelectionSet){
    _selectedRegion = region;
  }
  if(tools->selectionMode() == EditorTools::SelectionSubtract){
    _selectedRegion -= region;
  }
}


void ImageEditorView::setEditorMode(ImageEditorView::EditorMode new_mode){
  mode = new_mode;
  if(mode == EditorEditMaskMode){
    setCursor(QCursor(editMaskCursor));
  }
}

EditorTools * ImageEditorView::editorTools(){
  if(editorWorkspace){
    return editorWorkspace->editorTools();
  }
  return NULL;
}


void ImageEditorView::editMaskAt(QPoint pos){
  qDebug("Here");
  int radius = editorTools()->editMaskBrushRadius();
  /* Blur the area around the press */
  QList<QGraphicsItem *> it = items(pos);
  for(int i = 0; i < it.size(); i++){
    ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it.at(i));
    if(ii){
      QList<QPoint> points = ii->imagePointsAround(mapToScene(pos),radius);
      if(editorTools()->editMaskMode() == EditorTools::IncludeInMask){
	for(int i = 0;i<points.size();i++){
	  ii->setImageMask(points.at(i),1);
	}
      }else if(editorTools()->editMaskMode() ==  EditorTools::ExcludeFromMask){
	for(int i = 0;i<points.size();i++){
	  ii->setImageMask(points.at(i),0);
	}
      }
      ii->updateImage();
      emit imageItemChanged(ii);  
    }
  }
}


void ImageEditorView::updateEditMaskCursor(int radius){
  editMaskCursor = QPixmap((radius)*2+2,(radius)*2+2);
  editMaskCursor.fill(Qt::transparent);

  QPainter painter(&editMaskCursor);
  painter.setRenderHints(QPainter::Antialiasing);
  painter.drawEllipse(QRect(1,1,(radius)*2,(radius)*2));
  if(editorMode() == EditorEditMaskMode){
    setCursor(QCursor(editMaskCursor));
  }
}
