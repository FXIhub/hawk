#ifndef _PROCESSDISPLAY_H_
#define _PROCESSDISPLAY_H_

#include <QWidget>

class QPushButton;

class ProcessDisplay : public QWidget
{
  Q_OBJECT
    public:
  ProcessDisplay(QWidget * parent = NULL);
  void toggleRunButton(bool toggle);
  public slots:
  void onRunButtonToggled(bool toggled);
 signals:
  void runButtonToggled(bool toggled);
 private:
  bool processRunning;
  QPushButton * runButton;
};

#endif
