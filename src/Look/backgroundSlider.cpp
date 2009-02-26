#include <QtGui>
#include <spimage.h>
//#include "imageview.moc"
#include "backgroundSlider.h"

BackgroundSlider::BackgroundSlider(real value, QWidget *parent) : QDialog(parent)
{
  slider = new QSlider(Qt::Vertical);
  slider->setMinimumHeight(400);
  slider->setMaximumWidth(50);
  slider->setRange(0,1000);
  slider->setValue(500);
  min = 0.0;
  max = 2.0;

  minBox = new QDoubleSpinBox();
  minBox->setValue(0.0);
  maxBox = new QDoubleSpinBox();
  maxBox->setValue(2*value);
  levelBox = new QDoubleSpinBox();

  close = new QPushButton(tr("&Close"));

  connect(minBox, SIGNAL(valueChanged(double)), this, SLOT(minChanged(double)));
  connect(maxBox, SIGNAL(valueChanged(double)), this, SLOT(maxChanged(double)));
  connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
  connect(close, SIGNAL(clicked()), this, SLOT(accept()));
  connect(levelBox, SIGNAL(valueChanged(double)), this, SLOT(levelBoxChanged(double)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(maxBox);
  layout->addWidget(slider);
  layout->addWidget(minBox);
  layout->addWidget(close);
  layout->addWidget(levelBox);
  setLayout(layout);
}

void BackgroundSlider::minChanged(double value)
{
  real prev = (real) slider->value() / 1000.0 * (max - min) + min;
  if (value < max) min = value;
  if (prev < min) slider->setValue(0);
  else slider->setValue((int)((prev - min) * 1000.0 / (max - min)));
}

void BackgroundSlider::maxChanged(double value)
{
  real prev = (real) slider->value() / 1000.0 * (max - min) + min;
  if (value > min) max = value;
  if (prev > max) slider->setValue(1000);
  else slider->setValue((int)((prev-min) * 1000.0 / (max - min)));
}

void BackgroundSlider::sliderChanged(int value)
{
  //emit valueChanged((real) value / 1000.0 * (max - min) + min);
  levelBox->setValue((real) value / 1000.0 * (max - min) + min);
  /*
  printf("value = %i",value);
  printf("max = %g\n",max);
  printf("min = %g\n",min);
  printf("emitted %g\n",(real) value / 1000.0 * (max - min) + min);
  */
}

void BackgroundSlider::levelBoxChanged(double value)
{
  /*
  if (value < min) min = value;
  if (value > max) max = value;
  */

  slider->blockSignals(true);
  slider->setValue((int)((value-min) * 1000.0 / (max - min)));
  slider->blockSignals(false);

  emit valueChanged(value);
}

void BackgroundSlider::setValue(real value)
{
  minBox->setValue(0.0);
  maxBox->setValue((2*value));
  sliderChanged((int)value);
  levelBox->setValue(value);
}
