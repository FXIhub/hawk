#ifndef _PREVIEW_H_
#define _PREVIEW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN


#include "ui_preview.h"
#include <spimage.h>
#include <QDir>
#include <QTimer>

class Preview: public QMainWindow, private Ui::Preview
{
  Q_OBJECT
    public:
  Preview(QMainWindow *parent = 0);
 private:  
  Image * img;
  unsigned char * colormap_data;
  QDir dir;
 private slots:
  void on_imageRange_sliderReleased();
  void on_actionOpen_Directory_triggered(bool checked=false);
  void on_filesList_itemSelectionChanged();
  void openImage(QString filename);
  void loadNextImage();
  void on_logScaleCheckBox_stateChanged(int state);
  void loadImageComment(QString filename);
  void on_actionCopyHit_triggered(bool checked);
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
