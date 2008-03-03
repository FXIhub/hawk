#if defined NETWORK_SUPPORT
#include "configuration.h"
#include "client_decoder.h"
#include "uwrapc.h"

ClientDecoder::ClientDecoder(Communicator * p)
  :Decoder(p){
}

ClientDecoder * ClientDecoder::create(Communicator * p){
  return new ClientDecoder(p);
}


void ClientDecoder::execRun(QByteArray command){
  init_reconstruction(&global_options);
}

void ClientDecoder::setVariable(QByteArray command){
  /* remove the newline */
  command.chop(1);
  QList<QByteArray> tokens = command.split(' ');
  QString var(tokens.at(1));
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      if(variable_metadata[i].variable_type == Type_String){	
	strcpy((char *)(variable_metadata[i].variable_address),&(command.data()[command.indexOf(var)+var.size()+1]));
	qDebug("Set %s=%s",variable_metadata[i].variable_name,(char *)(variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_Real){
	if(tokens.size() != 3){
	  qFatal("Set real called without 3 arguments");
	}
	double r = tokens.at(2).toDouble();
	*((real *)variable_metadata[i].variable_address) = r;
	qDebug("Set %s=%f",variable_metadata[i].variable_name,r);
      }else if(variable_metadata[i].variable_type == Type_Int){
	if(tokens.size() != 3){
	  qFatal("Set int called without 3 arguments");
	}
        int r = tokens.at(2).toInt();
	*((int *)variable_metadata[i].variable_address) = r;
	qDebug("Set %s=%d",variable_metadata[i].variable_name,r);
      }else if(variable_metadata[i].variable_type == Type_Bool){
	if(tokens.size() != 3){
	  qFatal("Set bool called without 3 arguments");
	}
        int r = tokens.at(2).toInt();
	*((int *)variable_metadata[i].variable_address) = r;
	qDebug("Set %s=%d",variable_metadata[i].variable_name,r);
      }else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	int flag = 0;
	for(int j = 0;variable_metadata[i].list_valid_names[j];j++){
	  if(strcmp((char *)(variable_metadata[i].list_valid_names[j]),&(command.data()[command.indexOf(var)+var.size()+1])) == 0){
	    *((int *)variable_metadata[i].variable_address) = variable_metadata[i].list_valid_values[j];
	    flag = 1;
	    break;
	  }	  
	}
	if(!flag){
	  qFatal("MultipleChoice not valid!");
	}
	qDebug("Set %s=%d",variable_metadata[i].variable_name,*((int *)variable_metadata[i].variable_address));
      }
    }
  }
}

#endif
