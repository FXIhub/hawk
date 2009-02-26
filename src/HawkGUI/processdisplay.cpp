#include "processdisplay.h"
#include <QtGui>

ProcessDisplay::ProcessDisplay(QWidget * parent)
  :QWidget(parent)
{
  processRunning = false;
  QHBoxLayout *layout = new QHBoxLayout;
  setLayout(layout);
  layout->addStretch();
  runButton = new QPushButton(QIcon(":/images/exec.png"),"Run",this);
  layout->addWidget(runButton);
  runButton->setCheckable(true);
  layout->addStretch();
  connect(runButton,SIGNAL(toggled(bool)),this,SLOT(onRunButtonToggled(bool)));
}

void ProcessDisplay::onRunButtonToggled(bool toggled){
  if(toggled){
    runButton->setIcon(QIcon(":/images/stop.png"));
    runButton->setText("Stop");
  }else{
    runButton->setIcon(QIcon(":/images/exec.png"));
    runButton->setText("Run");
  }
  emit runButtonToggled(toggled);
}


void ProcessDisplay::toggleRunButton(bool toggle){
  runButton->setChecked(toggle);
}
