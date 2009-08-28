#ifndef _IMAGEVIEW_H_
#define _IMAGEVIEW_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QWidget>
#include <QPointF>
#include <QList>
#include <QGraphicsView>

class ImageItem;
class QGraphicsScene;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QFocusFrame;
class QMainWindow;
class ImageLoader;
class ImageViewPanel;

class ImageView: public QGraphicsView
{
  Q_OBJECT
    public:
  ImageView(QWidget * parent = NULL);
  ~ImageView();
  void setImage(ImageItem * item);
  void setup();
  void setAutoUpdate(bool update);  
  bool getAutoUpdate();
  QPointF getPos();
  QTransform getTransform();
  QString getFilename();
  QString scheduledFilename();
  QString currentlyLoadingFilename();
  QString getScheduledFilename();
  void setColormap(int color);
  int colormap();
  int display();
  QString newestFilename();
  QString getCurrentIteration();  
  void setDisplay(int display);
  bool logScale();
  ImageItem * selectedImage() const;
  ImageViewPanel * imageViewPanel() const;
  QString imageItemIdentifier(ImageItem * item);
  void showIdentifiers(bool show = true);
 public slots:
  void shiftImage();
  void loadUserSelectedImage();
  void scheduleImageLoad(QString file);
  virtual bool loadImage(QString file);
  void loadImage(QPixmap pix);
  void setPos(QPointF pos);
  void setTransform(QTransform t);
  void setLogScale(bool on);
  void maxContrast();
  void fourierTransform();
  void fourierTransformSquared();  
  void scaleItems(qreal scale);
  void translateItems(QPointF mov);
  void setPreserveShift(bool on);
  bool preservesShift() const;
  virtual void saveImage();
  void emitImageItemChanged(ImageItem * item);
 signals:
  void focusedIn(ImageView * focused);
  void scaleBy(qreal scale);
  void translateBy(QPointF r);
  void imageLoaded(QString file);
  void imageItemChanged(ImageItem * item);
 protected:
  void focusInEvent ( QFocusEvent * event );
  void mouseMoveEvent(QMouseEvent * event);
  void wheelEvent ( QWheelEvent * event );
  void mousePressEvent(QMouseEvent * event);
  void mouseReleaseEvent( QMouseEvent * mouseEvent );
  void keyPressEvent ( QKeyEvent * event );
  void mouseOverValue(QMouseEvent * event);
  QGraphicsScene * graphicsScene;
  ImageItem * _selected;
  bool _showIdentifiers;
  private slots:
  void finishLoadImage();
  void loadScheduledImage();
 private:
  QString positionToIdentifier(int p);
  QPointF mouseLastScenePos;
  ImageItem * dragged;
  QPointF draggedInitialPos;
  QPointF itemsScale;
  bool mouseInsideImage;
  QFocusFrame * focusFrame;
  QString displayType;
  bool autoUpdate;
  QString filename;
  QString fileToRetry;
  QMainWindow * mainWindow;
  ImageLoader * loader;
  QTimer * delayedLoader;
  QString scheduledImage;
  QString currentlyLoading;
  QString currentIteration;
  ImageViewPanel * panel;
  bool preserveShift;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
