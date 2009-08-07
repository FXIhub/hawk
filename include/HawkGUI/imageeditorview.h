#ifndef _IMAGEEDITORVIEW_H_
#define _IMAGEEDITORVIEW_H_
#if defined __cplusplus || defined Q_MOC_RUN

#include "imageview.h"

#include <QPointF>

class ImageEditorView: public ImageView
{
  Q_OBJECT
  Q_PROPERTY(QPointF HawkImage_imageCenter READ imageCenter WRITE setImageCenter)
  Q_PROPERTY(QSize HawkImage_pixelDimensions READ pixelDimensions)
  Q_PROPERTY(bool HawkImage_phased READ phased WRITE setPhased)
  Q_PROPERTY(bool HawkImage_scaled READ scaled WRITE setScaled)
  Q_PROPERTY(bool HawkImage_shifted READ shifted WRITE setShifted)
  Q_PROPERTY(double HawkImage_detectorDistance READ detectorDistance WRITE setDetectorDistance)    
    public:
  ImageEditorView(QWidget * parent = 0);
  void setImageCenter(QPointF center);
  QPointF imageCenter() const;
  QSize pixelDimensions() const;
  QString propertyNameToDisplayName(QString propertyName);
  bool phased() const;
  void setPhased(bool p);
  bool scaled() const;
  void setScaled(bool p);
  bool shifted() const;
  void setShifted(bool p);
  double detectorDistance() const;
  void setDetectorDistance(double p);

};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
