#ifndef _IMAGEEDITORVIEW_H_
#define _IMAGEEDITORVIEW_H_
#if defined __cplusplus || defined Q_MOC_RUN

#include "imageview.h"
#include <spimage.h>

#include <QPointF>
class QMouseEvent;
class QRubberBand;
class EditorWorkspace;

typedef enum{EditorDefaultMode = 0,EditorBlurMode = 1,EditorSelectionMode,EditorLineoutMode,EditorBullseyeMode} EditorMode;

class ImageEditorView: public ImageView
{
  Q_OBJECT
    Q_PROPERTY(QPointF HawkImage_imageCenter READ imageCenter WRITE setImageCenter)
  Q_PROPERTY(QSize HawkImage_imageSize READ imageSize WRITE setImageSize)
    Q_PROPERTY(QSizeF HawkImage_pixelSize READ pixelSize WRITE setPixelSize)
  Q_PROPERTY(bool HawkImage_phased READ phased WRITE setPhased)
  Q_PROPERTY(bool HawkImage_scaled READ scaled WRITE setScaled)
  Q_PROPERTY(bool HawkImage_shifted READ shifted WRITE setShifted)
  Q_PROPERTY(double HawkImage_detectorDistance READ detectorDistance WRITE setDetectorDistance)    
  Q_PROPERTY(double HawkImage_wavelength READ wavelength WRITE setWavelength)
    public:
  ImageEditorView(QWidget * parent,EditorWorkspace * workspace);
  void setImageCenter(QPointF center);
  QPointF imageCenter() const;
  QSize imageSize() const;
  void setImageSize(QSize imageSize);
  bool phased() const;
  void setPhased(bool p);
  bool scaled() const;
  void setScaled(bool p);
  bool shifted() const;
  void setShifted(bool p);
  double wavelength() const;
  void setWavelength(double w);
  double detectorDistance() const;
  void setDetectorDistance(double p);
  QSizeF pixelSize() const;
  void setPixelSize(QSizeF pixelSize);
  EditorMode editorMode();
  double getDropBrushRadius();
  double getDropBlurRadius();
  QRegion selectedRegion();
  void setBullseyeMode(bool on);
  void selectRegion(QRegion region);
 public slots:
  void setBlurMode();
  void setSelectionMode();
  void setLineoutMode();
  void setDefaultMode();
  void setDropBrushRadius(double d);
  void setDropBlurRadius(double d);
 protected:
  void paintEvent(QPaintEvent * e);
  void mouseReleaseEvent(QMouseEvent * e);
  void mousePressEvent(QMouseEvent * e);
  void mouseMoveEvent(QMouseEvent * e);
  void wheelEvent(QWheelEvent * e);
 private:
  Image * getBlurKernel();
  void generateDropCursor();
  EditorMode mode;
  double dropBrushRadius;
  double dropBlurRadius;
  QPixmap dropCursor;
  QRubberBand * rubberBand;
  QPoint rubberBandOrigin;  
  QPoint lineOutOrigin;
  QPoint lineOutEnd;
  QRegion _selectedRegion;
  EditorWorkspace * editorWorkspace;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
