#include "imageItem.h"
#include "workspace.h"
#include "server.h"
#include <spimage.h>

Workspace::Workspace(QWidget * parent)
  :QWidget(parent)
{
  setupUi(this);
  hboxLayout1->setMargin(0);
  hboxLayout->setMargin(0);
  setupViewers();
  colormap_flags = COLOR_JET;
  colormap_min = 0;
  colormap_max = 0;
  image = 0;
 }


void Workspace::setupViewers(){
  preprocessViewer->setSceneRect(QRect(0,0,400,400));
  /*  preprocessViewer->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);*/
  preprocessViewer->setBackgroundBrush(QBrush(QColor(Qt::black)));
  preprocessScene = new ImageViewer(preprocessViewer);
  preprocessScene->setSceneRect(QRect(-100000,-100000,200000,200000));
  preprocessViewer->setScene(preprocessScene);    
}

void Workspace::loadImage(QString filename){
  Image * tmp = sp_image_read(filename.toAscii(),0);
  if(!tmp){
    qDebug(("Failed to read image " + filename).toAscii());
    return;
  }
  if(image){
    sp_image_free(image);
  }
  image = tmp;
  colormap_data = sp_image_get_false_color(image,colormap_flags,colormap_min,colormap_max);
  qimage = QImage(colormap_data,sp_image_x(image),sp_image_y(image),QImage::Format_RGB32);
  ImageItem * item = new ImageItem(QPixmap::fromImage(qimage));
  item->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsFocusable);
  preprocessScene->addImage(item);    
}
