#include <QMessageBox>
#include "../configuration.h"
#include "server_decoder.h"

ServerDecoder::ServerDecoder(Communicator * p)
  :Decoder(p){
}

ServerDecoder::~ServerDecoder(){
  
}

ServerDecoder * ServerDecoder::create(Communicator * p){
  return new ServerDecoder(p);
}

void ServerDecoder::setVariable(QByteArray command){
  /* remove the newline */
  command.chop(1);
  QList<QByteArray> tokens = command.split(' ');
  QString var(tokens.at(1));
  for(int i = 0;i<number_of_global_options;i++){
    if(var.compare(variable_metadata[i].variable_name) == 0){
      if(variable_metadata[i].variable_type == Type_String){	
	strcpy((char *)(variable_metadata[i].variable_address),&(command.data()[command.indexOf(var)+var.size()+1]));
      }else if(variable_metadata[i].variable_type == Type_Real){
	if(tokens.size() != 3){
	  qFatal("Set real called without 3 arguments");
	}
	double r = tokens.at(2).toDouble();
	*((real *)variable_metadata[i].variable_address) = r;
      }else if(variable_metadata[i].variable_type == Type_Int){
	if(tokens.size() != 3){
	  qFatal("Set int called without 3 arguments");
	}
        int r = tokens.at(2).toInt();
	*((int *)variable_metadata[i].variable_address) = r;
      }else if(variable_metadata[i].variable_type == Type_Bool){
	if(tokens.size() != 3){
	  qFatal("Set bool called without 3 arguments");
	}
        int r = tokens.at(2).toInt();
	*((int *)variable_metadata[i].variable_address) = r;
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
      }
    }
  }
}

