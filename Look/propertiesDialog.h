#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QtGui>
#include <spimage.h>

class PropertiesDialog : public QDialog
{
  Q_OBJECT
    public:
  PropertiesDialog(double *lambda, double *distance, double *x, double *y, bool notFirst, QWidget *parent = 0);
  
  public slots:
    void close();

 private:
  double *localLambda;
  double *localDistance;
  double *localX;
  double *localY;

  QDoubleSpinBox *wavelength;
  QDoubleSpinBox *detectorDistance;
  QDoubleSpinBox *detectorX;
  QDoubleSpinBox *detectorY;
};

#endif
