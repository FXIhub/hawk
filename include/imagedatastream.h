#ifndef _IMAGEDATASTREAM_H_ 
#define _IMAGEDATASTREAM_H_ 1

#if defined __cplusplus || defined Q_MOC_RUN
#include <QDataStream>

class ImageDataStream: public QDataStream QObject
{
  Q_OBJECT
    public:
  ImageDataStream()
    :QDataStream(){
  }
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
