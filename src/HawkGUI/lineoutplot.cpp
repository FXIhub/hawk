#include "lineoutplot.h"
#include <spimage.h>
#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#if QWT_VERSION >= 0x060000 || QWT_VERSION < 0x050000
#error Hawk requires Qwt 5.x
#endif

LineOutPlot::LineOutPlot(const Image * a, QLineF line)
  :QWidget(0)
{
  QGridLayout * grid = new QGridLayout;
  setLayout(grid);
  image = a;
  npoints = 100;
  xData = (double *)malloc(sizeof(double)*npoints);
  yData = (double *)malloc(sizeof(double)*npoints);
  sampleImage(line);
  QWidget * plot = plotLineOut();
  grid->addWidget(plot,0,0);
  grid->addWidget(new QLabel(QString("Length: %1 pixels\t Angle: %2 degrees").arg(line.length()).arg(line.angle()),this),1,0);
  resize(600,400);
  show();
}

void LineOutPlot::sampleImage(QLineF line){
  for(int i = 0;i<npoints;i++){
    real x = line.x1() + i*(line.x2()-line.x1())/npoints;
    real y = line.y1() + i*(line.y2()-line.y1())/npoints;
    xData[i] = line.length()*i/npoints;
    if(sp_image_contains_coordinates(image,x,y,0)){
      yData[i] = sp_image_interp(image,x,y,0);
    }else{
      yData[i] = 0;
    }
  }
}

QWidget * LineOutPlot::plotLineOut(){
  QwtPlot * plot = new QwtPlot(this);
  plot->setTitle("Image Line Out");
  plot->setAxisTitle(QwtPlot::yLeft, "Magnitude");
  QwtPlotCurve *curve = new QwtPlotCurve("lineout");
#if QT_VERSION >= 0x040000
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
  curve->setPen(QPen(Qt::black));
  curve->attach(plot);
  curve->setRawData(xData,yData,npoints);
  plot->setCanvasBackground(Qt::white);
  return plot;
}
