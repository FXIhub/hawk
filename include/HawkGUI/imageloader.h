#ifndef _IMAGELOADER_H_
#define _IMAGELOADER_H_ 1

#include <QThread>
#include <QMutex>
#include "spimage.h"


class ImageLoader: public QThread
{
  Q_OBJECT
 public:
  ImageLoader(QString file,QObject * parent = 0 );
  void run();
  Image * getImage();
  QString getFile();
 private:
  void loadImage(QString file);
  QString p_file;
  Image * p_image;
  static QMutex loading;
};

#endif
