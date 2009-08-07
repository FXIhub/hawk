#include "imageeditorview.h"
#include "imageitem.h"

ImageEditorView::ImageEditorView(QWidget * parent)
  :ImageView(parent)
{
  setPreserveShift(false);
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

QSize ImageEditorView::imageSize() const{
  QSize ret(0,0);
  if(imageItem() && imageItem()->getImage()){
    Image * a =  imageItem()->getImage();
    ret = QSize(sp_image_x(a),sp_image_y(a));
  }
  return ret;
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


