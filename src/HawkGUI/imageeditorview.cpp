#include "imageeditorview.h"
#include "imageitem.h"

ImageEditorView::ImageEditorView(QWidget * parent)
  :ImageView(parent)
{
}

void ImageEditorView::setImageCenter(QPointF center){
  if(imageItem && imageItem->getImage()){
    Image * a =  imageItem->getImage();
    a->detector->image_center[0] = center.x();
    a->detector->image_center[1] = center.y();    
  }
}

QPointF ImageEditorView::imageCenter() const{
  QPointF ret(0,0);
  if(imageItem && imageItem->getImage()){
    Image * a =  imageItem->getImage();
    ret = QPointF(a->detector->image_center[0],a->detector->image_center[1]);
  }
  return ret;
}

QSize ImageEditorView::pixelDimensions() const{
  QSize ret(0,0);
  if(imageItem && imageItem->getImage()){
    Image * a =  imageItem->getImage();
    ret = QSize(sp_image_x(a),sp_image_y(a));
  }
  return ret;
}


bool ImageEditorView::phased() const{
  bool phased = false;
  if(imageItem && imageItem->getImage()){
    Image * a =  imageItem->getImage();
    phased = a->phased;
  }
  return phased;
}

void ImageEditorView::setPhased(bool phased){
  if(imageItem && imageItem->getImage()){
    Image * a =  imageItem->getImage();
    a->phased = phased;
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


