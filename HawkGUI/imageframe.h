#ifndef _IMAGEFRAME_H_
#define _IMAGEFRAME_H_ 1

#include <QWidget>
#include <QList>
#include <QFileInfo>
#include <QMap>

class ImageView;
class QLabel;
class ImageCategory;
class QHBoxLayout;
class QComboBox;
class ImageDisplay;

class ImageFrame: public QWidget
{
  Q_OBJECT
  public:
  ImageFrame(ImageView * view, ImageDisplay * parent);
  void setImageCategories(QList<ImageCategory *> * ic);
  QFileInfo getNextFile(QString file);
  QFileInfo getPreviousFile(QString file);
public slots:
  void onImageLoaded(QString file);  
protected:
  void keyPressEvent ( QKeyEvent * event );
private slots:
  void onCategoryBoxChanged(int index);
  void onImageLoadingTimerTimeout();
  private:
  void setTitle(QString s);
  QString incrementFilename(QString filename, int increment) const; 
  int discoverIncrement(QFileInfo fi) const;
  ImageView * imageView;
  QLabel * title;
  QList<ImageCategory *> * imageCategories;
  QHBoxLayout * topLayout;
  QComboBox * categoryBox;
  ImageDisplay * imageDisplay;
  QString filename;
  QTimer * imageLoadingTimer;
  QMap <QString,int> suggestedIncrement;
};

#endif
