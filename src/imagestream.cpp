#include "imagestream.h"


void ImageStream::setMaskCompression(bool on, int level){
  if(on){
    m_maskCompression = level;
  }else{
    m_maskCompression = 0;
  }
}

ImageStream & ImageStream::operator<<(const sp_i3matrix * mask){
  QDataStream & s = *this;
  QByteArray compressedMask = qCompress((const uchar *)mask->data,
					 sp_i3matrix_size(mask)*sizeof(int),m_maskCompression);
  qDebug("Compressed i3matrix from %lld to %d bytes",sp_i3matrix_size(mask)*sizeof(int),compressedMask.size());
  s.writeBytes(compressedMask.constData(),compressedMask.size());
  return *this;
}

ImageStream & ImageStream::operator>>(sp_i3matrix * & mask){
  QDataStream & s = *this;
  uchar * buffer;
  uint l;
  s.readBytes((char *&)buffer,l);
  QByteArray imageMask = qUncompress((uchar *)buffer,l);
  l = imageMask.size();
  memcpy(mask->data,imageMask.constData(),imageMask.size());
  free(buffer);
  uint expectedSize = sp_i3matrix_size(mask)*sizeof(int);
  if(l != expectedSize){
    qWarning("ImageStream: read %u instead of %u bytes",l,expectedSize);
  }else{
    qDebug("ImageStream: correctly read %u bytes",l);
  }
  return *this;
}

ImageStream & ImageStream::operator<<(const sp_c3matrix * image){
  QDataStream & s = *this;
  s.writeBytes((const char *)image->data,sp_c3matrix_size(image)*sizeof(real)*2);
  return *this;
}

ImageStream & ImageStream::operator>>(sp_c3matrix * & image){
  QDataStream & s = *this;
  uint l;
  uint expectedSize = sp_c3matrix_size(image)*sizeof(real)*2;    
  sp_free(image->data);
  s.readBytes((char *&)image->data,l);
  if(l != expectedSize){
    qWarning("ImageStream: read %u instead of %u bytes",l,expectedSize);
  }else{
    qDebug("ImageStream: correctly read %u bytes",l);
  }
  return *this;
}

ImageStream & ImageStream::operator<<(const Detector * d){
  QDataStream & s = *this;
  s << d->image_center[0] << d->image_center[1] << d->image_center[2];
  s << d->pixel_size[0] << d->pixel_size[1] << d->pixel_size[2];
  s << d->detector_distance;
  s << d->wavelength;
  return *this;
}

ImageStream & ImageStream::operator>>(Detector * & d){
  QDataStream & s = *this;
  s >> d->image_center[0] >> d->image_center[1] >> d->image_center[2];
  s >> d->pixel_size[0] >> d->pixel_size[1] >> d->pixel_size[2];
  s >> d->detector_distance;
  s >> d->wavelength;
  return *this;
}

/*ImageStream & ImageStream::operator<<(Image * a){
  return *this << (const Image *)a;
  }*/

ImageStream & ImageStream::operator<<(const Image * a){
  ImageStream & s = *this;
  qDebug("ImageStream: serializing image at %p",a); 
  s << sp_image_x(a) << sp_image_y(a) << sp_image_z(a);
  s << a->phased;
  s << a->scaled;
  s << (int)QSysInfo::ByteOrder;
  s << (int)sizeof(real);
  //  qDebug("Image size %d %d %d",x,y,z);
  s << a->image;
  s << a->mask;
  s << a->detector;
  s << a->shifted;
  return *this;
}

ImageStream & ImageStream::operator>>(Image * & a){
  ImageStream & s = *this;
  qDebug("ImageStream: deserializing image at %p",a); 
  int x,y,z;
  s>> x >> y >> z;
  qDebug("Image size %d %d %d",x,y,z);
  a = sp_image_alloc(x,y,z);
  s >> a->phased;
  s >> a->scaled;
  QSysInfo::Endian sourceOrder;
  int i;
  s >> i;
  sourceOrder = (QSysInfo::Endian)i;
  if(sourceOrder != QSysInfo::ByteOrder){
    qCritical("ImageStream: transfering data across system with different endianess not implemented!");
    return *this;
  }
  int sourceRealSize;
  s >> sourceRealSize;
  if(sourceRealSize != sizeof(real)){
    qCritical("ImageStream: transfering data across system with different sizeof(real) not implemented!");
    return *this;
  }
  /* the byte order of the source
     is the same as ours, no need for conversion */
  s >> a->image;
  s >> a->mask;
  s >> a->detector;
  s >> i;
  a->shifted = i;
  return *this;
}
