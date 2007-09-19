#include <QQueue>
#include <QTextStream>
#include <QTimer>
#include <QByteArray>
#include <QMutexLocker>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "communicator.h"


#ifdef _SP_DOUBLE_PRECISION
typedef double real;
#else
typedef float real;
#endif


typedef enum {Type_Real=0, Type_Int, Type_String, 
	      Type_MultipleChoice,Type_Bool, Type_Group}Variable_Type;

typedef enum {Id_Diffraction_Filename=0,Id_Real_Image_Filename,Id_Max_Blur_Radius,Id_Init_Level,
	      Id_Beta,Id_Iterations,Id_Support_Mask_Filename,Id_Init_Support_Filename,Id_Image_Guess_Filename,
	      Id_Noise,Id_Beamstop,Id_New_Level
	      ,Id_Iterations_To_Min_Blur,Id_Blur_Radius_Reduction_Method,Id_Min_Blur,
	      Id_Enforce_Real,Id_Log_File,Id_Commandline,Id_Output_Period,Id_Log_Output_Period,
	      Id_Algorithm,Id_Exp_Sigma,Id_Dyn_Beta,Id_Rand_Phases,Id_Rand_Intensities,Id_Cur_Iteration,
	      Id_Adapt_Thres,Id_Automatic,Id_Work_Dir,Id_Real_Error_Threshold,Id_Support_Update_Algorithm,
	      Id_Output_Precision,Id_Error_Reduction_Iterations_After_Loop,Id_Enforce_Positivity,
	      Id_Genetic_Optimization,Id_Charge_Flip_Sigma,Id_Rescale_Amplitudes,Id_Square_Mask,
	      Id_Patterson_Blur_Radius,Id_Remove_Central_Pixel_Phase,Id_Perturb_Weak_Reflections,Id_Nthreads,
	      Id_Break_Centrosym_Period,Id_Reconstruction_Finished,Id_Real_Error_Tolerance,Id_Root,
	      Id_Remove_Central_Pixel_phase,Id_Max_Iterations,Id_Patterson_Level_Algorithm,Id_Object_Area,
	      Id_Image_Blur_Period,Id_Image_Blur_Radius,Id_Iterations_To_Min_Object_Area
}Variable_Id;


typedef enum {isSettableBeforeRun = 1, isSettableDuringRun = 2, isGettableBeforeRun = 4,
				 isGettableDuringRun = 8, isMandatory = 16} Variable_Properties;
typedef struct VariableMetadata{
  const char * variable_name;
  const Variable_Type variable_type;
  const Variable_Id id;
  const struct VariableMetadata * parent;
  const Variable_Properties variable_properties;
  const int list_valid_values[10];
  /* No more than 10 possible values per list */
  const char * list_valid_names[10];
  const void * variable_address;
  /* We should also have a documentation field */
}VariableMetadata;

extern const int number_of_global_options;
extern const VariableMetadata variable_metadata[100];

QMutex Decoder::socketReadMutex;

Decoder::Decoder(Communicator * p){
  parent = p;
}

void Decoder::run(){
  socketReadMutex.lock();
  if(!parent->commandQueue.size()){
    /* no data to read, just return */
    socketReadMutex.unlock();
    return;
  }
  QByteArray command = parent->commandQueue.dequeue();
  if(command.startsWith("Data ")){
    QList<QByteArray> tokens = command.split(' ');
    if(tokens.size() != 3){
      fprintf(stderr,"Data command does not contain 3 parts! Discarding data!\n");
      socketReadMutex.unlock();
      return;
    }      
    QString dataId(tokens.at(1));
    qulonglong dataSize(tokens.at(2).trimmed().toULongLong());
    qulonglong dataRead = 0;
    QByteArray data;
    while(dataRead < dataSize){
      if(parent->commandQueue.size() > 0){
	QByteArray new_data = parent->commandQueue.dequeue();
	data.append(new_data);
	dataRead = data.size();
      }
    };    
    /* chop newline out */
    data.chop(1);
  }else if(command.startsWith("Set ")){
    setVariable(command);
  }
  socketReadMutex.unlock();
}

void Decoder::setVariable(QByteArray command){
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
      }
    }
  }
}

Communicator::Communicator(QTcpSocket * s)
{
  socket = s;
  connect(socket,SIGNAL(readyRead()),this,SLOT(readCommands()));
}



void Communicator::readCommands(){
  /* Make sure we finish handling any command before we 
     allow anyone else to mess with out datastream*/
  while(socket->canReadLine()){
    QByteArray command = socket->readLine();    
    commandQueue.enqueue(command);    
    Decoder * dec = new Decoder(this);
    dec->start();
    connect(dec,SIGNAL(finished()),this,SLOT(cleanupDecoders()));
    decoderList.append(dec);
  }    
}



void Communicator::cleanupDecoders(){
  for (int i = 0; i < decoderList.size(); ++i) {
    if (decoderList.at(i)->isFinished()){
      Decoder * old = decoderList.takeAt(i);
      delete old;
    }
  }
}
