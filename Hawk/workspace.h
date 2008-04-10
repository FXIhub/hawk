#ifndef _WORKSPACE_H_
#define _WORKSPACE_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QMainWindow>
#include <QGraphicsScene>
#include "ui_workspace.h"
#include <spimage.h>
#include "imageViewer.h"

class MainWindow;

class Workspace: public QWidget , private Ui::Workspace
{
  Q_OBJECT
    public:
  Workspace(QWidget * parent = NULL,MainWindow * main=NULL);
 public slots:
  void loadImage(QString filename);
 private:
  void setupViewers();
  ImageViewer * preprocessScene;
  Image * image;
  uchar * colormap_data;
  int colormap_flags;
  real colormap_min;
  real colormap_max;
  QImage qimage;  
  MainWindow * mainWindow;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
