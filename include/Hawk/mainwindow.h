#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QMainWindow>
#include "ui_Hawk.h"
#include "applicationmode.h"

class Server;
class Workspace;
class ImageItem;

class MainWindow: public QMainWindow, private Ui::Hawk
{
  Q_OBJECT
    public:
  MainWindow(QMainWindow * parent = NULL);
 public slots:
  void setSelectedImageItem(ImageItem * it);
  ImageItem * selectedImageItem(){
    return selImageItem;
  }
  void fillImagePropertiesTable(ImageItem * it);
  void imageItemChanged(ImageItem * it);
 signals:
  void modeChanged(ApplicationMode mode);
  void selectedImageItemChanged(ImageItem * it);
 private:
  Server * server;
  QList<Workspace *> workspaces;
  ImageItem * selImageItem;
 private slots:
  void on_actionOpenImage_triggered(bool checked);
  void on_actionExcludeFromMask_toggled(bool checked);
  void on_actionPickCenter_toggled(bool checked);
  void on_actionIncludeInMask_toggled(bool checked);
  void on_actionNavigate_toggled(bool checked);
  void on_actionExit_triggered(bool checked);
  void on_actionSaveImage_triggered(bool checked);

};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
