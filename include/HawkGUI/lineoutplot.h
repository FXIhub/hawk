#ifndef _LINEOUTPLOT_H_
#define _LINEOUTPLOT_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QWidget>
#include <spimage.h>

class LineOutPlot: public QWidget
{
  Q_OBJECT
    public:
  LineOutPlot(Image * a, QLineF line);
 private:
  void sampleImage(QLineF line);
  void plotLineOut();
  double * xData;
  double * yData;
  int npoints;
  Image * image;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
