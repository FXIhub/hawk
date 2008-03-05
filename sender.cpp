#if defined NETWORK_SUPPORT
#include "sender.h"
#include "configuration.h"
#include "communicator.h"
#include <QTcpSocket>
#include <QDataStream>


Sender::Sender(QTcpSocket * s)
{
  socket = s;
}


void Sender::setCommunicator(Communicator * c){
  communicator = c;
}


void Sender::sendSetVariable(QString variable_name,int value){
  QString var = variable_name;
  QByteArray message;
  message.append("Set ");
  int outer_flag = 0;
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      outer_flag = 1;
      if(variable_metadata[i].variable_type == Type_String){	
	qFatal("Received int value when it expected a string");
      }else if(variable_metadata[i].variable_type == Type_Real || variable_metadata[i].variable_type == Type_Int){
	message.append(variable_name);
	message.append(QString(" %d\n").arg(value));	
	break;
      }else if(variable_metadata[i].variable_type == Type_Bool){
	if(value != 1 && value != 0){
	  qFatal("Received int value when it expected a bool");
	}
	message.append(variable_name);
	message.append(QString(" %d\n").arg(value));	
	break;
      }else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	int flag = 0;
	for(int j = 0;variable_metadata[i].list_valid_names[j];j++){
	  if(value == variable_metadata[i].list_valid_values[j]){
	    message.append(variable_name);
	    message.append(" ");
	    message.append(QString(variable_metadata[i].list_valid_names[j]));
	    flag = 1;
	    break;
	  }
	}
	if(!flag){
	  qFatal("MultipleChoice not valid!");
	}
      }
      break;
    }
  }  
  if(!outer_flag){
    qFatal("Variable name not found!");
  }
  int size = (message.size());
  message.prepend(QByteArray::fromRawData((char *)&size,sizeof(int)));
  emit writeToSocket(message);
}

void Sender::sendSetVariable(QString variable_name,double value){
  QString var = variable_name;
  int flag = 0;
  QByteArray message("Set ");
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      flag = 1;
      if(variable_metadata[i].variable_type == Type_Real){
	message.append(variable_name);
	message.append(QString(" %1\n").arg(value));	
      }else{
	qFatal("Did not received real when it expected one!");	
      }
      break;
    }
  }
  if(!flag){
    qFatal("Variable name not found!");
  }
  int size = (message.size());
  message.prepend(QByteArray::fromRawData((char *)&size,sizeof(int)));

  emit writeToSocket(message);	
}

void Sender::sendSetVariable(QString variable_name,QString value){
  QString var = variable_name;
  int flag = 0;
  QByteArray message("Set ");
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      flag = 1;
      if(variable_metadata[i].variable_type == Type_String){
	message.append(variable_name);
	message.append(" ");
	message.append(value);
	break;
      }else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	int multi_flag = 0;
	for(int j = 0;variable_metadata[i].list_valid_names[j];j++){
	  if(value.compare(((char *)(variable_metadata[i].list_valid_names[j]))) == 0){
	    message.append(variable_name);
	    message.append(QString(" %d\n").arg(variable_metadata[i].list_valid_values[j]));	
	    multi_flag = 1;
	    break;
	  }	  
	}
	if(!multi_flag){
	  qFatal("MultipleChoice not valid!");
	}
      }else{
	qFatal("Did not received a string when it expected one!");	
      }
      break;
    }
  }
  if(!flag){
    qFatal("Variable name not found!");
  }
  int size =(message.size());
  message.prepend(QByteArray::fromRawData((char *)&size,sizeof(int)));
  emit writeToSocket(message);
}

void Sender::sendSetVariable(QString variable_name,Image * a){
  QString var = variable_name;
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
  sendData(vm,a);
}

void Sender::sendExecGetVariable(QString variable_name){
  QString var = variable_name;
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
  int id = communicator->insertIdPair(vm);
  QByteArray message("Exec Get ");
  message.append(variable_name);
  message.append(QString(" %1\n").arg(id));	
  int size = message.size();
  message.prepend(QByteArray::fromRawData((char *)&size,sizeof(int)));

  emit writeToSocket(message);
  /* Now the handler for the Data reply should take care of telling the main App that new data is ready to be read */
}


void Sender::sendData(VariableMetadata * vm, int id){
  QByteArray message("Data ");
  int size;

  message.append(QString::number(vm->variable_type));
  message.append(" ");
  if(id < 0){
    message.append(vm->variable_name);
  }else{
    message.append(QString::number(id));
  }
  message.append(" ");
  QByteArray data;
  QDataStream out(&data, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_0);
  if(vm->variable_type == Type_Real){
    qreal value = *((real *)vm->variable_address);
    out << value;
  }
  if(vm->variable_type == Type_Int || vm->variable_type == Type_MultipleChoice || vm->variable_type == Type_Bool){
    qint32 value = *((qint32 *)vm->variable_address);
    out << value;
  }else if(vm->variable_type == Type_String){
    QString value((char *)vm->variable_address);
    out << value;
  }else if(vm->variable_type == Type_Image){
    /* this depends on image.h! */
    Image * a = **((Image ***)vm->variable_address);
    if(a){
      out << 1;
      qint32 phased = a->phased;
      out << phased;
      qint32 scaled = a->scaled;
      out << scaled;
      if(a->image){
	out << 1;
	qint32 image_x = a->image->x;
	out << image_x;
	qint32 image_y = a->image->y;
	out << image_y;
	qint32 image_z = a->image->z;
	out << image_z;
	for(int i = 0;i<image_x*image_y*image_z;i++){
	  qreal re = sp_real(a->image->data[i]);
	  out << re;
	  qreal im = sp_imag(a->image->data[i]);
	  out << im;
	}
      }else{
	out << 0;
      }
      if(a->mask){
	out << 1;
	qint32 mask_x = a->mask->x;
	out << mask_x;
	qint32 mask_y = a->mask->y;
	out << mask_y;
	qint32 mask_z = a->mask->z;
	out << mask_z;
	for(int i = 0;i<mask_x*mask_y*mask_z;i++){
	  qint32 re= (a->mask->data[i]);
	  out << re;
	}
      }else{
	out << 0;
      }
      
      qreal image_center_x = a->detector->image_center[0];
      out << image_center_x;
      qreal image_center_y = a->detector->image_center[1];
      out << image_center_y;
      qreal image_center_z = a->detector->image_center[2];
      out << image_center_z;
      
      qreal pixel_size_x = a->detector->pixel_size[0];
      out << pixel_size_x;
      qreal pixel_size_y = a->detector->pixel_size[1];
      out << pixel_size_y;
      qreal pixel_size_z = a->detector->pixel_size[2];
      out << pixel_size_z;
      
      qreal detector_distance = a->detector->detector_distance;
      out << detector_distance;
      qreal lambda = a->detector->lambda;
      out << lambda;
      
      qint32 shifted = a->shifted;
      out << shifted;
      
      if(a->rec_coords){
	out << 1;
	qint32 rec_coords_x = a->rec_coords->x;
	out << rec_coords_x;
	qint32 rec_coords_y = a->rec_coords->y;
	out << rec_coords_y;
	qint32 rec_coords_z = a->rec_coords->z;
	out << rec_coords_z;
	for(int i = 0;i<rec_coords_x*rec_coords_y*rec_coords_z;i++){
	  qreal re= (a->rec_coords->data[i]);
	  out << re;
	}
      }else{
	out << 0;
      }
    }else{
      out << 0;
    }
    qint32 num_dimensions = a->num_dimensions;          
    out << num_dimensions;
  }

  size = data.size()+1;
  message.append(QString::number(size));
  message.append("\n");
  message.append(data);
  message.append("\n");
  int m_size = (message.size());
  message.prepend(QByteArray::fromRawData((char *)&m_size,sizeof(int)));

  emit writeToSocket(message);
}



void Sender::sendData(VariableMetadata * vm, Image * a){
  QByteArray message("Data ");
  int size;

  message.append(QString::number(vm->variable_type));
  message.append(" ");
  message.append(vm->variable_name);
  message.append(" ");
  QByteArray data;
  QDataStream out(&data, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_0);
  if(vm->variable_type == Type_Image){
    /* this depends on image.h! */
    qint32 phased = a->phased;
    out << phased;
    qint32 scaled = a->scaled;
    out << scaled;
    if(a->image){
      out << 1;
      qint32 image_x = a->image->x;
      out << image_x;
      qint32 image_y = a->image->y;
      out << image_y;
      qint32 image_z = a->image->z;
      out << image_z;
      for(int i = 0;i<image_x*image_y*image_z;i++){
	qreal re = sp_real(a->image->data[i]);
	out << re;
      qreal im = sp_imag(a->image->data[i]);
      out << im;
      }
    }else{
      out << 0;
    }
    if(a->mask){
      out << 1;
      qint32 mask_x = a->mask->x;
      out << mask_x;
      qint32 mask_y = a->mask->y;
      out << mask_y;
      qint32 mask_z = a->mask->z;
      out << mask_z;
      for(int i = 0;i<mask_x*mask_y*mask_z;i++){
	qint32 re= (a->mask->data[i]);
	out << re;
      }
    }else{
      out << 0;
    }

    qreal image_center_x = a->detector->image_center[0];
    out << image_center_x;
    qreal image_center_y = a->detector->image_center[1];
    out << image_center_y;
    qreal image_center_z = a->detector->image_center[2];
    out << image_center_z;

    qreal pixel_size_x = a->detector->pixel_size[0];
    out << pixel_size_x;
    qreal pixel_size_y = a->detector->pixel_size[1];
    out << pixel_size_y;
    qreal pixel_size_z = a->detector->pixel_size[2];
    out << pixel_size_z;
    
    qreal detector_distance = a->detector->detector_distance;
    out << detector_distance;
    qreal lambda = a->detector->lambda;
    out << lambda;

    qint32 shifted = a->shifted;
    out << shifted;
    
    if(a->rec_coords){
      out << 1;
      qint32 rec_coords_x = a->rec_coords->x;
      out << rec_coords_x;
      qint32 rec_coords_y = a->rec_coords->y;
      out << rec_coords_y;
      qint32 rec_coords_z = a->rec_coords->z;
      out << rec_coords_z;
      for(int i = 0;i<rec_coords_x*rec_coords_y*rec_coords_z;i++){
	qreal re= (a->rec_coords->data[i]);
	out << re;
      }
    }else{
      out << 0;
    }
    qint32 num_dimensions = a->num_dimensions;          
    out << num_dimensions;
  }else{
    qFatal("Data type not supported!");
  }
  size = data.size()+1;
  message.append(QString::number(size));
  message.append("\n");
  message.append(data);
  message.append("\n");
  int m_size = (message.size());
  message.prepend(QByteArray::fromRawData((char *)&m_size,sizeof(int)));

  emit writeToSocket(message);
}


void Sender::sendExecRun(){
  QByteArray message("Exec Run\n");
  int m_size = (message.size());
  message.prepend(QByteArray::fromRawData((char *)&m_size,sizeof(int)));
  emit writeToSocket(message);
}

#endif
