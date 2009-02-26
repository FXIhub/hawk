#ifndef CROPDIALOG_H
#define CROPDIALOG_H

#include <QtGui>
#include <spimage.h>

class CropDialog : public QDialog
{
  Q_OBJECT
    public:
  CropDialog(int *x1, int *x2, int *y1, int *y2, QWidget *parent = 0);
  
  public slots:
    void close();
  void update();

 private:
  int *localX1;
  int *localX2;
  int *localY1;
  int *localY2;

  QSpinBox *x1Box;
  QSpinBox *x2Box;
  QSpinBox *y1Box;
  QSpinBox *y2Box;
  QPushButton *OK;
};

#endif
