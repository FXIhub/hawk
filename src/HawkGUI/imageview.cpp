#include "imageview.h"
#include <QtGui>
#include "imageitem.h"
#include <QMainWindow>
#include "imageloader.h"
#include "imagecategory.h"
#include "imageviewpanel.h"

ImageView::ImageView(QWidget * parent)
  :QGraphicsView(parent)
{
  dragged = 0;
  autoUpdate = 1;
  myImageItem = NULL;
  preserveShift = true;
  setup();
  QWidgetList tlwidgets =  QApplication::topLevelWidgets();
  int size = tlwidgets.size();
  for(int i = 0;i<size;i++){
    mainWindow = qobject_cast<QMainWindow *>(tlwidgets.at(i));
    if(mainWindow){
      break;
    }
  }
  if(!mainWindow){
    qDebug("Could not find main Window");
  }else{
    mainWindow->statusBar()->showMessage(tr("Main Window Found"));
  }
  delayedLoader = new QTimer(this);
  connect(delayedLoader,SIGNAL(timeout()),this,SLOT(loadScheduledImage()));
  delayedLoader->start(3);
  loader = NULL;
  QVBoxLayout * vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0,0,0,0);
  setLayout(vbox);
  vbox->addStretch();
  panel = new ImageViewPanel(this);
  vbox->addWidget(panel);
  connect(this,SIGNAL(scaleBy(qreal)),this,SLOT(scaleItems(qreal)));
  connect(this,SIGNAL(translateBy(QPointF)),this,SLOT(translateItems(QPointF)));
}

ImageView::~ImageView()
{
  if(loader){
    // wait for loader to exit
    loader->wait();
  }
}


void ImageView::mousePressEvent(QMouseEvent * event){
  if(event->buttons() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      if(QString("ImageItem") == it[i]->data(0)){
	dragged = (ImageItem *)it[i];
	draggedInitialPos = it[i]->mapFromScene(mapToScene(event->pos()))-it[i]->pos();
	break;
      }
    }
    /*    if(dragged){
      setCursor(QCursor(Qt::OpenHandCursor));
    }else{
      setCursor(QCursor(Qt::ClosedHandCursor));
      } */   
  }
}

void ImageView::mouseReleaseEvent( QMouseEvent *  ){
  if(dragged){
    dragged = 0;
  }
}

void ImageView::mouseMoveEvent(QMouseEvent * event){
  if(dragged && event->buttons() & Qt::LeftButton){
    QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
    translateBy(mov);
  }else if(event->buttons() & Qt::LeftButton){
    QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
    emit translateBy(mov);
  }else if(event->buttons() & Qt::RightButton){  
    QPointF mouse_mov = mapToScene(event->pos())-mouseLastScenePos;
    qreal speed = 0.005;
    qreal scale = 1-mouse_mov.y()*speed;
    emit scaleBy(scale);
  }

  mouseInsideImage = false;
  QList<QGraphicsItem *> it = items(event->pos());
  for(int i = 0; i < it.size(); i++){
    mouseInsideImage = true;
    ImageItem * ii = qgraphicsitem_cast<ImageItem *>(it.at(i));
    if(ii){
      Image * image = ii->getImage();
      if(image){
	QPointF pos = it.at(i)->mapFromScene(mapToScene(event->pos()));    
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
	Complex v = sp_image_get(image,x,y,0);
	QString message = QString("Pixel %1,%2 Value=%3 + %4i Amp=%5 Phase=%6%7").arg(x).arg(y).arg(sp_real(v)).arg(sp_imag(v)).arg(sp_cabs(v)).arg(180/3.1415*sp_carg(v)).arg(QChar(0x00B0));
	mainWindow->statusBar()->showMessage(message);
	//	qDebug("Hovering over image at %dx%d",x,y);
      }
    }
    break;
  }
  mouseLastScenePos = mapToScene(event->pos());
  event->accept();
}

void ImageView::wheelEvent( QWheelEvent * event ){
  qreal speed = 0.0005;
  qreal scale = 1+event->delta()*speed;
  emit scaleBy(scale);
}

ImageItem * ImageView::imageItem() const{
  return myImageItem;
}

void ImageView::scaleItems(qreal scale){
  if(imageItem()){
    // Don't let the user zoom in or out too much 
    if((scale < 1 && scale > 0 && 
	imageItem()->getScale().x() > 0.01 &&
	imageItem()->getScale().y() > 0.01) ||
       (scale > 1 && 
	imageItem()->getScale().x() < 100 &&
	imageItem()->getScale().y() < 100)){
      QPointF screen_center = sceneRect().center();
      QList<QGraphicsItem *> it = items();
      for(int i = 0; i < it.size(); i++){
	QPointF item_sc = it[i]->mapFromScene(screen_center);
	it[i]->scale(scale, scale);
	QPointF item_a_sc = it[i]->mapFromScene(screen_center);
	QPointF mov = item_a_sc-item_sc;
	it[i]->translate(mov.x(),mov.y());
      }
    }
  }
}

void ImageView::translateItems(QPointF mov){
  if(imageItem()){
    imageItem()->moveBy(mov.x(),mov.y());
  }
}

void ImageView::setImage(ImageItem * item){
  int display = -1;
  int color = -1;
  bool isShifted = false;
  if(imageItem()){
    // If we already have an image loaded we're gonna preserve the location of the center
    // and the zoom
    item->setTransform(imageItem()->transform());
    QSizeF center_correction = imageItem()->boundingRect().size()/2-item->boundingRect().size()/2;
    item->setPos(imageItem()->pos()+QPointF(center_correction.width(),center_correction.height()));

    // and colormap and display
    color = imageItem()->colormap();
    display = imageItem()->display();
    isShifted = imageItem()->isShifted();
  }else{
    // Set pixmap center in the middle of the screen
    QPointF center = sceneRect().center();
    center.setX(center.x()-item->pixmap().width()*item->getScale().x()/2);
    center.setY(center.y()-item->pixmap().height()*item->getScale().y()/2);
    item->setPos(center);
  }
  delete myImageItem;
  myImageItem = item;
  if(preserveShift && isShifted != imageItem()->isShifted()){
    imageItem()->shiftImage();
  }
  if(color >= 0){
    imageItem()->setColormap(color);
  }
  if(display >= 0){
    imageItem()->setDisplay(display);
  }
  graphicsScene->clear();
  graphicsScene->addItem(item);  
  emit imageItemChanged(imageItem());
}

void ImageView::keyPressEvent ( QKeyEvent * event ){
  event->ignore();
  if(event->key() == Qt::Key_Plus){
    emit scaleBy(1.25);
    event->accept();
  }
  if(event->key() == Qt::Key_Minus){
    emit scaleBy(0.75);
    event->accept();
  }
  if(event->modifiers() & Qt::ControlModifier){
    if(event->key() == Qt::Key_Up){
      emit translateBy(QPointF(0,-10));
      event->accept();
    }
    if(event->key() == Qt::Key_Down){
      emit translateBy(QPointF(0,10));
      event->accept();
    }
    if(event->key() == Qt::Key_Right){
      emit translateBy(QPointF(10,0));
      event->accept();
    }
    
    if(event->key() == Qt::Key_Left){
      emit translateBy(QPointF(-10,0));
      event->accept();
    }
  }
}


void ImageView::setup(){
  QLinearGradient grad = QLinearGradient(0,0,0,500);
  grad.setColorAt(1,QColor("#B2DFEE"));
  grad.setColorAt(0,QColor("#26466D"));
  QBrush grad_brush(grad);      
  setSceneRect(QRect(0,0,width(),height()));
  setBackgroundBrush(grad_brush);
  graphicsScene = new QGraphicsScene(this);
  setScene(graphicsScene);    
}

void ImageView::loadUserSelectedImage(){
  QString fileName = QFileDialog::getOpenFileName(this, tr("Load Image"),
						  QString(),
						  tr("Images (*.h5 *.png *tif *tiff)"));
   if(!fileName.isEmpty()){
     loadImage(fileName);
   }
}


void ImageView::saveImage(){
  if(imageItem() && imageItem()->getImage()){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"));
    qDebug("Trying to save %s",fileName.toAscii().data());
    if(!fileName.isEmpty()){
      sp_image_write(imageItem()->getImage(),fileName.toAscii().data(),0);
    }
  }
}

bool ImageView::loadImage(QString file){
  if(loader == NULL){
    scheduledImage.clear();
    currentlyLoading = file;
    loader = new ImageLoader(file,this);
    connect(loader,SIGNAL(finished()),this,SLOT(finishLoadImage()));
    loader->start();
    return true;
  }else{
    scheduledImage = file;
    qDebug("Loader busy...");
    return false;
  }
}

void ImageView::loadImage(QPixmap pix){
  ImageItem * item = new ImageItem(pix,NULL);
  setImage(item);    
  item->update();

}

void ImageView::focusInEvent ( QFocusEvent * ){
  emit focusedIn(this);
}

QPointF ImageView::getPos(){
  return imageItem()->pos();
}

void ImageView::setPos(QPointF pos){
  imageItem()->setPos(pos);
}

QTransform ImageView::getTransform(){
  return imageItem()->transform();
}

void ImageView::setTransform(QTransform transform){
  imageItem()->setTransform(transform);
}

bool ImageView::getAutoUpdate(){
  return autoUpdate;
}

void ImageView::setAutoUpdate(bool update){
  autoUpdate = update;
}

QString ImageView::getFilename(){
  return filename;
}


void ImageView::shiftImage(){
  if(imageItem()){
    imageItem()->shiftImage();
    emit imageItemChanged(imageItem());
  }
}

void ImageView::fourierTransform(){
  if(imageItem()){
    imageItem()->fourierTransform((mapToScene(0,0,width(),height())).boundingRect(),false);
    emit imageItemChanged(imageItem());
  }
}

void ImageView::fourierTransformSquared(){
  if(imageItem()){
    imageItem()->fourierTransform((mapToScene(0,0,width(),height())).boundingRect(),true);
    emit imageItemChanged(imageItem());
  }
}


void ImageView::setColormap(int color){
  if(imageItem()){
    imageItem()->setColormap(color);
    emit imageItemChanged(imageItem());
  }
}

int ImageView::colormap(){
  if(imageItem()){
    return imageItem()->colormap();
  }
  return 0;
}

void ImageView::setDisplay(int display){
  if(imageItem()){
    imageItem()->setDisplay(display);
    emit imageItemChanged(imageItem());
  }
}

int ImageView::display(){
  if(imageItem()){
    return imageItem()->display();
  }
  return 0;
}

void ImageView::finishLoadImage(){
  loader = qobject_cast<ImageLoader *>(sender());
  Image * image = loader->getImage();
  if(!image){
    qDebug(("Failed to read image " + loader->getFile()).toAscii());
    return;
  }
  currentlyLoading.clear();
  filename = loader->getFile();
  currentIteration = ImageCategory::getFileIteration(filename);
  qDebug(("Current iteration:" + currentIteration).toAscii());
  delete loader;
  loader = NULL;
  ImageItem * item = new ImageItem(image,filename,NULL);
  setImage(item);    
  item->update();
  emit imageLoaded(filename);
}

void ImageView::scheduleImageLoad(QString file){
  scheduledImage = file;
}

void ImageView::loadScheduledImage(){
  //  qDebug("Checking scheduled image");
  if(!scheduledImage.isEmpty()){
    if(loader == NULL){
      QString file = scheduledImage;
      currentlyLoading = file;
      scheduledImage.clear();
      loader = new ImageLoader(file,this);
      connect(loader,SIGNAL(finished()),this,SLOT(finishLoadImage()));
      loader->start();
    }
  }
}

QString ImageView::scheduledFilename(){
  return scheduledImage;
}

QString ImageView::currentlyLoadingFilename(){
  return currentlyLoading;
}

QString ImageView::newestFilename(){
  if(!scheduledImage.isEmpty()){
    return scheduledImage;
  }
  if(!currentlyLoading.isEmpty()){
    return currentlyLoading;
  }
  return filename;
}

QString ImageView::getCurrentIteration(){
  return currentIteration;
}

void ImageView::maxContrast(){
  if(imageItem()){
    imageItem()->maxContrast((mapToScene(0,0,width(),height())).boundingRect());
    emit imageItemChanged(imageItem());
  }
}

void ImageView::setLogScale(bool on){
  if(imageItem()){
    imageItem()->setLogScale(on);
    emit imageItemChanged(imageItem());
  }
}

bool ImageView::logScale(){
  if(imageItem()){
    return imageItem()->logScale();
  }
  return false;
}


ImageViewPanel * ImageView::imageViewPanel() const{
  return panel;
}

void ImageView::setPreserveShift(bool on){
  preserveShift = on;
}

bool ImageView::preservesShift() const{
  return preserveShift;
}
