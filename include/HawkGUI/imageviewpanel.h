#ifndef _IMAGEVIEWPANEL_H_
#define _IMAGEVIEWPANEL_H_
#if defined __cplusplus || defined Q_MOC_RUN

#include <QWidget>

class ImageViewPanel: public QWidget
{
  Q_OBJECT
    public:
  ImageViewPanel(QWidget * parent = 0);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
