#ifndef BACKGROUNDSLIDER_H
#define BACKGROUNDSLIDER_H

#include <QtGui>
#include <spimage.h>

class BackgroundSlider : public QDialog
{
  Q_OBJECT
    public:
  BackgroundSlider(real value, QWidget *parent = 0);
  void setValue(real value);
  
 signals:
  void valueChanged(real);

  public slots:
    void minChanged(double value);
  void maxChanged(double value);
  void sliderChanged(int value);
  void levelBoxChanged(double value);
  //void close();

 private:
  QSlider *slider;
  QDoubleSpinBox *maxBox;
  QDoubleSpinBox *minBox;
  QPushButton * close;
  QDoubleSpinBox *levelBox;

  real min, max;
};

#endif
