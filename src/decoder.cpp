#if defined NETWORK_SUPPORT
#include <QMutex>
#include <QTime>
#include <QDataStream>
#include "decoder.h"
#include "communicator.h"


Decoder::Decoder(Communicator * p){
  parent = p;
}

Decoder * Decoder::create(Communicator * p){
  return new Decoder(p);
}

void Decoder::run(){
  if(!parent->commandQueue.size()){
    /* no data to read, just return */
    return;
  }
  QTime t;
  t.start();
  QByteArray command = parent->commandQueue.dequeue();
  if(command.startsWith("Data ")){
    readData(command);
  }else if(command.startsWith("Set ")){
    setVariable(command);
  }else if(command.startsWith("Exec Get")){
    execGet(command);
  }else if(command.startsWith("Exec Run")){
    execRun(command);
  }
  qDebug("Time it took to handle command: %d ms",t.elapsed());
}

void Decoder::execGet(QByteArray command){
  QList<QByteArray> tokens = command.split(' ');
  if(tokens.size() != 4){
    fprintf(stderr,"Data command does not contain 4 parts! Discarding data!\n");
    return;
  }   
  QString var(tokens.at(2));
  int dataId(tokens.at(3).trimmed().toInt());
  VariableMetadata * vm = NULL;
  int flag = 0;
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      vm = (VariableMetadata *)&(variable_metadata[i]);
      flag = 1;
      break;
    }
  }
  if(!flag){
    qFatal("Variable name not found!");
  }
  if(((vm->variable_properties & isGettableDuringRun) && global_options.is_running) ||
     ((vm->variable_properties & isGettableBeforeRun) && !global_options.is_running)){
    parent->sendData(vm,dataId);
  }else{
    qDebug("Request for ungettable data");
  }
}
 
void Decoder::readData(QByteArray command_and_data){
  QByteArray command = command_and_data.mid(0,command_and_data.indexOf("\n"));
  QList<QByteArray> tokens = command.split(' ');
  command_and_data.remove(0,command_and_data.indexOf("\n")+1);
  command_and_data.chop(1);
			  
  if(tokens.size() != 4){
    fprintf(stderr,"Data command does not contain 4 parts! Discarding data!\n");
    return;
  }      
  bool idIsNumeric;
  int dataType(tokens.at(1).trimmed().toInt());
  int dataId(tokens.at(2).trimmed().toInt(&idIsNumeric));
  if(!idIsNumeric){
    /* dataType is actually a name*/
    QString var = tokens.at(2).trimmed();
    int flag = 0;
    VariableMetadata * vm = NULL;
    for(int i = 0;i<number_of_global_options;i++){
      if(var.compare(variable_metadata[i].variable_name) == 0){
	vm = (VariableMetadata *)&(variable_metadata[i]);
	flag = 1;
	break;
      }
    }
    if(!flag){
      qFatal("Variable name not found!");
    }
    /* Make up a temporary id */
    dataId = parent->insertIdPair(vm);
  } 
  qulonglong dataSize(tokens.at(3).trimmed().toULongLong());
  if(dataSize-1 != (qulonglong)command_and_data.size()){
    qDebug("Data size internal check mismatch!");
  }
  QDataStream in(command_and_data);
  in.setVersion(QDataStream::Qt_4_0);
  if(dataType == Type_Real){
    qreal value;
    in >> value;
    *((real *)parent->getMetadataById(dataId)->variable_address) = value;
  }else if(dataType == Type_Int || dataType == Type_MultipleChoice || dataType == Type_Bool){
    qint32 value;
    in >> value;
    *((int *)parent->getMetadataById(dataId)->variable_address) = value;
  }else if(dataType == Type_String){
    QString value;
    in >> value;
    strcpy((char *)parent->getMetadataById(dataId)->variable_address,value.toAscii());
  }else if(dataType == Type_Image){
    /* this depends on image.h! */
    Image * a = NULL;
    qint32 imageExistFlag;
    in >> imageExistFlag;
    if(imageExistFlag){
      qint32 phased;
      in >> phased;
      qint32 scaled;
      in >> scaled;
      qint32 imageFlag;
      in >> imageFlag;
      if(imageFlag){
	qint32 image_x;
	in >> image_x;
	qint32 image_y;
	in >> image_y;
	qint32 image_z;
	in >> image_z;
	a = sp_image_alloc(image_x,image_y,image_z);
	for(int i = 0;i<image_x*image_y*image_z;i++){
	  qreal re = sp_real(a->image->data[i]);
	  in >> re;
	  sp_real(a->image->data[i]) = re;
	  qreal im = sp_imag(a->image->data[i]);
	  in >> im;
	  sp_imag(a->image->data[i]) = im;
	}
      }else{
	qFatal("Transmiting image with NULL pointer");
      }
      qint32 maskFlag;
      in >> maskFlag;
      
      if(maskFlag){
	qint32 mask_x;	
	in >> mask_x;
	qint32 mask_y;
	in >> mask_y;
	qint32 mask_z;
	in >> mask_z;
	for(int i = 0;i<mask_x*mask_y*mask_z;i++){
	  qint32 re;
	  in >> re;
	  a->mask->data[i] = re;
	}
	
      }
      qreal image_center_x; 
      in >> image_center_x;
      a->detector->image_center[0] = image_center_x;
      qreal image_center_y;
      in >> image_center_y;
      a->detector->image_center[1] = image_center_y;
      qreal image_center_z;
      in >> image_center_z;
      a->detector->image_center[2] = image_center_z;
      
      qreal pixel_size_x;
      in >> pixel_size_x;
      a->detector->pixel_size[0] = pixel_size_x;
      qreal pixel_size_y;
      in >> pixel_size_y;
      a->detector->pixel_size[1] = pixel_size_y;
      qreal pixel_size_z;
      in >> pixel_size_z;
      a->detector->pixel_size[2] = pixel_size_z;
      
      qreal detector_distance = a->detector->detector_distance;
      in >> detector_distance;
      a->detector->detector_distance = detector_distance;
      qreal lambda;
      in >> lambda;    
      a->detector->lambda = lambda;
      
      qint32 shifted;
      in >> shifted;
      a->shifted = shifted;
      
      qint32 rec_coordsFlag;
      in >> rec_coordsFlag;
      if(rec_coordsFlag){
	qint32 rec_coords_x;
	in >> rec_coords_x;
	qint32 rec_coords_y;
	in >> rec_coords_y;
	qint32 rec_coords_z;
	in >> rec_coords_z;
	a->rec_coords = sp_3matrix_alloc(rec_coords_x,rec_coords_y,rec_coords_z);      
	for(int i = 0;i<rec_coords_x*rec_coords_y*rec_coords_z;i++){
	  qreal re;
	  in >> re;
	  a->rec_coords->data[i] = re;
	}
      }
      qint32 num_dimensions;
      in >> num_dimensions;
      a->num_dimensions = (Dimensions)num_dimensions;
    }else{
      a = NULL;
      qDebug("Receiving null image!");
    }
    Image ** p = (Image **)sp_malloc(sizeof(Image *));
    p = &a;
    parent->getMetadataById(dataId)->variable_address = p;

  }
  emit dataReceived(dataId);    
}

void Decoder::execRun(QByteArray command){
}

void Decoder::readImage(QString id, qulonglong sizeRead, QByteArray data){
}

void Decoder::setVariable(QByteArray command){
}

#endif
