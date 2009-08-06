#ifndef _IMAGEEDITORVIEW_H_
#define _IMAGEEDITORVIEW_H_
#if defined __cplusplus || defined Q_MOC_RUN

#include "imageview.h"

#include <QPointF>

class ImageEditorView: public ImageView
{
  Q_OBJECT
  Q_PROPERTY(QPointF imageCenter READ imageCenter WRITE setImageCenter)
    public:
  ImageEditorView(QWidget * parent = 0);
  void setImageCenter(QPointF center);
  QPointF imageCenter() const;
  QString propertyNameToDisplayName(QString propertyName);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
