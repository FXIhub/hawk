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

QString ImageEditorView::propertyNameToDisplayName(QString propertyName){
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
