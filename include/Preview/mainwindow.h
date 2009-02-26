#ifndef _PREVIEW_H_
#define _PREVIEW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN


#include "ui_preview.h"

class Preview: public QWidget, private Ui::Preview
{
  Q_OBJECT
    public:
  Preview(QWidget *parent = 0);
 private:  
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
