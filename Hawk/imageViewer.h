#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGraphicsView>
#include <QPointF>
#include <QList>
#include "applicationmode.h"

class MainWindow;
class ImageBay;
class ImageItem;
class QGraphicsScene;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;


class ImageViewer: public QGraphicsView
{
  Q_OBJECT
    public:
  ImageViewer(QWidget * parent);
  void mouseMoveEvent(QMouseEvent * event);
  void wheelEvent ( QWheelEvent * event );
  void mousePressEvent(QMouseEvent * event);
  void mouseReleaseEvent( QMouseEvent * mouseEvent );
  void addImage(ImageItem * item);
  void keyPressEvent ( QKeyEvent * event );
  void keyReleaseEvent ( QKeyEvent * event );
  void scaleItems(qreal scale);
  void translateItems(QPointF mov);
  void createPreprocessBays();
  void setup(MainWindow * main);
  ApplicationMode getMode();
 public slots:
  void setMode(ApplicationMode newMode);
  void setModeCursor();
  void loadImage(QString filename);
 private:
  QPointF mouseLastScenePos;
  QGraphicsScene * graphicsScene;
  ImageItem * dragged;
  QPointF draggedInitialPos;
  QPointF itemsScale;
  QList<ImageBay *>bayList;
  QList<ImageItem *>imageItems;
  ApplicationMode mode;
  ApplicationMode alternateMode;
  MainWindow * mainWindow;
  bool controlDown;
  bool mouseInsideImage;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
