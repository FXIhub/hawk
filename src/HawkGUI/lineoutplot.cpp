#include "lineoutplot.h"
#include <spimage.h>
#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

LineOutPlot::LineOutPlot(Image * a, QLineF line)
  :QWidget(0)
{
  setLayout(new QGridLayout);
  image = a;
  npoints = 100;
  xData = (double *)malloc(sizeof(double)*npoints);
  yData = (double *)malloc(sizeof(double)*npoints);
  sampleImage(line);
  plotLineOut();
  resize(600,400);
  show();
}

void LineOutPlot::sampleImage(QLineF line){
  for(int i = 0;i<npoints;i++){
    real x = line.x1() + i*(line.x2()-line.x1())/npoints;
    real y = line.y1() + i*(line.y2()-line.y1())/npoints;
    xData[i] = i;
    if(sp_image_contains_coordinates(image,x,y,0)){
      yData[i] = sp_image_interp(image,x,y,0);
    }else{
      yData[i] = 0;
    }
  }
}

void LineOutPlot::plotLineOut(){
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
  layout()->addWidget(plot);
}
