#include <QtGui>
#include <spimage.h>
//#include "imageview.moc"
#include "cropDialog.h"

CropDialog::CropDialog(int *x1, int *x2, int *y1, int *y2, QWidget *parent)
{
  localX1 = x1;
  localX2 = x2;
  localY1 = y1;
  localY2 = y2;

  x1Box = new QSpinBox();
  x2Box = new QSpinBox();
  y1Box = new QSpinBox();
  y2Box = new QSpinBox();
  OK = new QPushButton(tr("&OK"));

  QLabel *x1Label = new QLabel("Low x [pixels]:");
  QLabel *x2Label = new QLabel("High x [pixels]:");
  //QLabel *detectorSizeLabel = new QLabel("Detector size:");
  QLabel *y1Label = new QLabel("Low y [pixels]:");
  QLabel *y2Label = new QLabel("High y [pixels]:");
  QPushButton *Cancel = new QPushButton(tr("&Cancel"));

  x1Box->setMaximum(*x2);
  x2Box->setMaximum(*x2);
  y1Box->setMaximum(*y2);
  y2Box->setMaximum(*y2);


  x1Box->setValue(*x1);
  x2Box->setValue(*x2);
  y1Box->setValue(*y1);
  y2Box->setValue(*y2);

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(x1Label,0,0);
  layout->addWidget(x2Label,1,0);
  layout->addWidget(y1Label,2,0);
  layout->addWidget(y2Label,3,0);
  layout->addWidget(x1Box,0,1);
  layout->addWidget(x2Box,1,1);
  layout->addWidget(y1Box,2,1);
  layout->addWidget(y2Box,3,1);
  layout->addWidget(Cancel,4,0);
  layout->addWidget(OK,4,1);
  setLayout(layout);

  connect(OK, SIGNAL(clicked()), this, SLOT(close()));
  connect(Cancel,SIGNAL(clicked()), this, SLOT(reject()));
  connect(x1Box, SIGNAL(valueChanged(int)), this, SLOT(update()));
  connect(x2Box, SIGNAL(valueChanged(int)), this, SLOT(update()));
  connect(y1Box, SIGNAL(valueChanged(int)), this, SLOT(update()));
  connect(y2Box, SIGNAL(valueChanged(int)), this, SLOT(update()));
}

void CropDialog::close()
{
  if (x2Box->value() - x1Box->value() == y2Box->value() - y1Box->value()) {
    *localX1 = x1Box->value();
    *localX2 = x2Box->value();
    *localY1 = y1Box->value();
    *localY2 = y2Box->value();
    emit accept();
  }
}

void CropDialog::update()
{
  if (x2Box->value() - x1Box->value() != y2Box->value() - y1Box->value() &&
      x2Box->value() > x1Box->value() && y2Box->value() > y1Box->value()) {
    OK->setText("n.p.");
  } else {
    OK->setText("OK");
  }
}
