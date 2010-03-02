#ifndef _IMAGESTREAM_H_ 
#define _IMAGESTREAM_H_ 1

#if defined __cplusplus || defined Q_MOC_RUN
#include <QDataStream>
#include <spimage.h>

class ImageStream: public QDataStream{
 public:
 ImageStream(QByteArray * b, QIODevice::OpenMode mode)
   :QDataStream(b,mode){
    m_maskCompression = -1;
  }
  ImageStream & operator<<(const Detector * d);
  ImageStream & operator>>(Detector *& d);
  ImageStream & operator<<(const Image * a);
  ImageStream & operator>>(Image *& a);
  ImageStream & operator<<(const sp_c3matrix * data);
  ImageStream & operator>>(sp_c3matrix *& data);
  ImageStream & operator<<(const sp_i3matrix * mask);
  ImageStream & operator>>(sp_i3matrix *& mask);
  ImageStream & operator>>(int & i){
    QDataStream::operator>>((qint32&)i);
    return *this;
  }
  ImageStream & operator<<(int i){
    QDataStream::operator<<((qint32)i);
    return *this;
  }
  void setMaskCompression(bool on,int level = -1);
 private:
  int m_maskCompression;
};
#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
