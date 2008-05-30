#include <QtGui>
#include <spimage.h>
//#include "imageview.moc"
#include "propertiesDialog.h"

PropertiesDialog::PropertiesDialog(real *lambda, real *distance, real *x, real *y, bool notFirst, QWidget *parent)
{
  localLambda = lambda;
  localDistance = distance;
  localX = x;
  localY = y;

  wavelength = new QDoubleSpinBox();
  detectorDistance = new QDoubleSpinBox();
  detectorX = new QDoubleSpinBox();
  detectorY = new QDoubleSpinBox();

  QLabel *wavelengthLabel = new QLabel("Wavelength [nm]:");
  QLabel *detectorDistanceLabel = new QLabel("Detector distance[mm]:");
  //QLabel *detectorSizeLabel = new QLabel("Detector size:");
  QLabel *detectorXLabel = new QLabel("Detector size X [mm]:");
  QLabel *detectorYLabel = new QLabel("Detector size Y [mm]:");
  QPushButton *OK = new QPushButton(tr("&OK"));

  if (notFirst) {
    printf("not first time\n");
    wavelength->setValue(*lambda);
    detectorDistance->setValue(*distance);
    detectorX->setValue(*x);
    detectorY->setValue(*y);
  } else
    printf("first time\n");


  QGridLayout *layout = new QGridLayout;
  layout->addWidget(wavelengthLabel,0,0);
  layout->addWidget(wavelength,0,1);
  layout->addWidget(detectorDistanceLabel,1,0);
  layout->addWidget(detectorDistance,1,1);
  //layout->addWidget(detectorSizeLabel,2,0);
  layout->addWidget(detectorXLabel,2,0);
  layout->addWidget(detectorX,2,1);
  layout->addWidget(detectorYLabel,3,0);
  layout->addWidget(detectorY,3,1);
  layout->addWidget(OK,4,1);
  setLayout(layout);

  connect(OK, SIGNAL(clicked()), this, SLOT(close()));
}

void PropertiesDialog::close()
{
  *localLambda = wavelength->value();
  *localDistance = detectorDistance->value();
  *localX = detectorX->value();
  *localY = detectorY->value();
  emit accept();
}
