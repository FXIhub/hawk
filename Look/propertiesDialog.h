#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QtGui>
#include <spimage.h>

class PropertiesDialog : public QDialog
{
  Q_OBJECT
    public:
  PropertiesDialog(real *lambda, real *distance, real *x, real *y, bool notFirst, QWidget *parent = 0);
  
  public slots:
    void close();

 private:
  real *localLambda;
  real *localDistance;
  real *localX;
  real *localY;

  QDoubleSpinBox *wavelength;
  QDoubleSpinBox *detectorDistance;
  QDoubleSpinBox *detectorX;
  QDoubleSpinBox *detectorY;
};

#endif
