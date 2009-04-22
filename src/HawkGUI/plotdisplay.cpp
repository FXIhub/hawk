#include "plotdisplay.h"
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_canvas.h>
#include <qpaintengine.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <QFileInfo>
#include <QLayout>
#include <QScrollBar>
#include <QFontMetrics>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include "logtailer.h"

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
	//	setTrackerMode(QwtPlotZoomer::AlwaysOn);
	//        setTrackerMode(QwtPicker::AlwaysOff);
	//        setRubberBand(QwtPicker::NoRubberBand);
	QPen pen = rubberBandPen();
	pen.setColor(Qt::black);
	setRubberBandPen(pen);
	setTrackerPen(pen);
        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,
            Qt::RightButton);
    }
};

CurveData::CurveData()
{
  d_count = 0;
}

void CurveData::append(double *newX, double *newY, int newCount)
{
  int newSize = ( (count() + newCount) / 1000 + 1 ) * 1000;
    if ( newSize > size() )
    {
        d_x.resize(newSize);
        d_y.resize(newSize);
    }

    for (int i = 0; i < newCount; i++ )
    {
      d_x[count() + i] = newX[i];
      d_y[count() + i] = newY[i];
    }
    d_count += newCount;
}

int CurveData::count() const
{
    return d_count;
}

int CurveData::size() const
{
    return d_x.size();
}

const double *CurveData::x() const
{
    return d_x.data();
}

const double *CurveData::y() const
{
    return d_y.data();
}

PlotDisplay::PlotDisplay(QWidget * parent)
  :QwtPlot(parent)
{

  logTailer = NULL;

  setAutoReplot(false);
  QLinearGradient grad = QLinearGradient(0,0,0,500);
  grad.setColorAt(1,QColor("#B2DFEE"));
  grad.setColorAt(0,QColor("#26466D"));
  QBrush grad_brush(grad);      

  QPalette palette;
  palette.setBrush(QPalette::Window,grad_brush);
  canvas()->setPalette(palette);
  //  setCanvasBackground(QColor(29, 100, 141));
  //  setAxisScale(xBottom, 0, 10);
  setAxisAutoScale(xBottom);
  setAxisAutoScale(yLeft);
  setMargin(18);

  zoomer = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft,canvas());
  zoomer->setRubberBandPen(QPen(Qt::black, 2, Qt::DotLine));
  zoomer->setTrackerPen(QPen(Qt::black));
  // legend
  QwtLegend *legend = new QwtLegend;
  legend->setFrameStyle(QFrame::StyledPanel|QFrame::Raised);
  insertLegend(legend, QwtPlot::LeftLegend);
  legend->setItemMode(QwtLegend::CheckableItem);
  legend->contentsWidget()->layout()->setSpacing(0);
  legend->contentsWidget()->layout()->setContentsMargins(0,0,0,0);


  legend->setLineWidth(0);
  connect(this,SIGNAL(legendChecked(QwtPlotItem *,bool)),this,SLOT(setCurveVisible(QwtPlotItem *,bool)));
  dirty = false;
  connect(&updateTimer,SIGNAL(timeout()),this,SLOT(updateCurves()));
  updateTimer.start(6000);
}

PlotDisplay::~PlotDisplay(){
  QList<CurveData * > data = d_datasets.values();
  int size = data.size();
  for(int i = 0;i<size;i++){
    delete data.at(i);
  }
}

void PlotDisplay::createDataset(QString name,int id){
  d_datasets.insert(id,new CurveData);
  QFontMetrics fm(font());
  // QwtPlotCurve * curve = new QwtPlotCurve(fm.elidedText(name,Qt::ElideRight,100));
  QwtPlotCurve * curve = new QwtPlotCurve(name);
  curve->setStyle(QwtPlotCurve::Lines);
  curve->setPaintAttribute(QwtPlotCurve::PaintFiltered);
  curve->setPaintAttribute(QwtPlotCurve::ClipPolygons);
  /*  curve->setRenderHint(QwtPlotItem::RenderAntialiased);    */
  curve->attach(this);  
  QwtLegendItem * li = (QwtLegendItem *)(legend()->find(curve));
  li->setChecked(true);
  li->setToolTip(name);
#if QWT_VERSION < 0x050200
  li->setIdentfierWidth(18);
#else
  li->setIdentifierWidth(18);
#endif
  d_curves.insert(id,curve);
  QVector<QColor> colors = generatePlotColors(d_curves.size());
  QList<QwtPlotCurve *> curves = d_curves.values();
  int size = curves.size();
  for(int i = 0;i<size;i++){
    QPen pen = QPen(colors.at(i),2);
    switch(i%5){
    case 0:
      pen.setStyle(Qt::SolidLine);
      break;
    case 1:
      {
	QVector<qreal> dashes;
	qreal space = 3;
	dashes << 3 << space;
	pen.setDashPattern(dashes);
      }
      //      pen.setStyle(Qt::DashLine);
      break;
    case 2:
      {
	QVector<qreal> dashes;
	qreal space = 3;
	dashes << 1 << space;
	pen.setDashPattern(dashes);
      }
      break;
    }
    pen.setJoinStyle(Qt::RoundJoin);

    curves.at(i)->setPen(pen);
  }
  Q_ASSERT(d_datasets.size() == d_curves.size());
  dirty = true;
}

int PlotDisplay::appendData(DatasetId id, double x, double y){
  int ret = appendData(id,&x, &y, 1);
  return ret;
}

int PlotDisplay::appendData(DatasetId id, double *x, double *y, int size){
  CurveData * data = datasetById(id);
  if ( data == NULL ){
    return -1;
  }
  QwtPlotCurve * curve = curveById(id);
  if ( curve == NULL ){
    return -1;
  }
  data->append(x, y, size);
  dirty = true;
  return 0;
}


CurveData * PlotDisplay::datasetById(DatasetId id){
  return d_datasets.value(id);
}

QwtPlotCurve * PlotDisplay::curveById(DatasetId id){
  return d_curves.value(id);
}

int PlotDisplay::setCurveVisible(DatasetId id,bool visible){
  QwtPlotCurve * curve = curveById(id);
  if ( curve == NULL ){
    return -1;
  }  
  setCurveVisible(curve,visible);
}

int PlotDisplay::setCurveVisible(QwtPlotItem *plotItem, bool visible){
  plotItem->setVisible(visible);
  QwtLegendItem * li = (QwtLegendItem *)legend()->find(plotItem);
  Q_ASSERT(li != NULL);
  li->setChecked(visible);
  dirty = true;

  updateCurves();
  updateZoomBase();
  replot();
  return 0;
}


void PlotDisplay::onProcessStarted(QString type, QString path,ProcessControl * p){    
  //  qDebug("creating log tailer");
  if(logTailer){
    delete logTailer;
  }
  logTailer = new LogTailer(this);

  clearPlot();
  replot();
  
  logTailer->tailLogFile(path+"/uwrapc.log");
  connect(logTailer,SIGNAL(dataLineRead(QList<double>)),this,SLOT(addDataLine(QList<double>)));
  connect(logTailer,SIGNAL(headerRead(QString,int)),this,SLOT(addHeader(QString,int)));
}

void PlotDisplay::addDataLine(QList<double> data){
  //  qDebug("new data");
  int size = data.size();
  for(int i = 1;i<size;i++){
    int id = i-1;
    if(datasetById(id)){
      appendData(id,data.at(0),data.at(i));
    }
  }
}

void PlotDisplay::addHeader(QString title, int col){
  title.chop(3);
    title.replace(" ","\n");
  if(curveById(col)){
    // curve already exists
    curveById(col)->setTitle(title);
    return;
  }
  createDataset(title,col);
  if(title == "Ereal" ||
     title == "Efourier" ||
     title == "SupSize(%)"){
    setCurveVisible(col,true);
  }else{
    setCurveVisible(col,false);
  }
}

void PlotDisplay::updateZoomBase(){
  QList<QwtPlotCurve *> curves = d_curves.values();
  QList<CurveData *> data = d_datasets.values();
  int size = curves.size();
  QwtDoubleRect rect;
  for(int i = 0;i<size;i++){
    if(curves.at(i)->isVisible()){
      QwtDoubleRect bounding = curves.at(i)->boundingRect();
      if(bounding.width() == 0){
	bounding.setWidth(0.1);
	bounding.moveLeft(bounding.left()-0.05);
      }
      if(bounding.height() == 0){
	bounding.setHeight(0.1);
	bounding.moveTop(bounding.top()-0.05);
      }
      if(rect.isEmpty()){
	rect = bounding;
      }else{
	if(!bounding.isEmpty()){
	  rect = rect.united(bounding);
	}
      }
    }
  }
  if(!rect.isEmpty()){
    int zoomer_at_top = 0;
    if(zoomer->zoomRectIndex() == 0){
      zoomer_at_top = 1;
    }

    if(rect.contains(zoomer->zoomBase())){
      //      qDebug("Normal update rect");
      zoomer->setZoomBase(rect);      
    }else{
      //      qDebug("Shrinking rect");
      zoomer->zoom(rect);
      zoomer->setZoomBase(rect);
    }
    if(zoomer_at_top){
      zoomer->zoom(0);
    }
  }else{
    //    qDebug("Empty rect");
  }
}


void PlotDisplay::updateCurves(){
  if(dirty){
    QList<QwtPlotCurve * > curves = d_curves.values();
    QList<CurveData * > datas = d_datasets.values();
    int size = curves.size();
    for(int i = 0;i<size;i++){
      QwtPlotCurve * curve = curves.at(i);
      CurveData * data = datas.at(i);
      if(!curve->isVisible()){
	continue;
      }
      curve->setRawData(data->x(), data->y(), data->count());
      
      const bool cacheMode = 
	canvas()->testPaintAttribute(QwtPlotCanvas::PaintCached);
      
      const bool oldDirectPaint = 
	canvas()->testAttribute(Qt::WA_PaintOutsidePaintEvent);
      
      const QPaintEngine *pe = canvas()->paintEngine();
      if(!pe){	
	/* For some reason there's no paintEngine still? */
	return;
      }
      bool directPaint = pe->hasFeature(QPaintEngine::PaintOutsidePaintEvent);
      if ( pe->type() == QPaintEngine::X11 ){
	// Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent 
	// works on X11. This has an tremendous effect on the performance..
	directPaint = true;
      }
      canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, directPaint);
      
      canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
      curve->draw(curve->dataSize() - size, curve->dataSize() - 1);
      canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);
      
      canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, oldDirectPaint);
      

    }
    updateZoomBase();
    dirty = false;
  }
}


QVector<QColor> PlotDisplay::generatePlotColors(int noColors){
  QColor bg = QColor("#26466D");
  QColor fg = QColor("#B2DFEE");

  if((fg.hue() - bg.hue()+360)%360 < (bg.hue() - fg.hue()+360)%360){
    QColor swap = bg;
    bg = fg;
    fg = swap;
  }
  QVector<QColor> colors;
  
  //  qDebug("bg hue = %d fg hue = %d",bg.hue(),fg.hue());
  for (int i = 1; i < noColors+1; i++) {
    int h,s,v;
    h = int(bg.hue() + ((360.0 -(bg.hue() - fg.hue()))/ (noColors+1)* i)) % 360;
    s = 240;
    v = int(qMax(bg.value(), fg.value()) * 0.85);
    v = 255;
    //    qDebug("appending with hue = %d",h);
    colors.append(QColor::fromHsv(h, s, v));
  }  
  return colors;
}

void PlotDisplay::loadUserSelectedLogFile(){
  QString fileName = QFileDialog::getOpenFileName(this, tr("Load log file"),
						  QString(),
						  tr("Log (*.log)"));
  if(!fileName.isEmpty()){
    if(logTailer){
      delete logTailer;
    }
    logTailer = new LogTailer(this);
    
    clearPlot();
    replot();
  
    logTailer->tailLogFile(fileName);
    connect(logTailer,SIGNAL(dataLineRead(QList<double>)),this,SLOT(addDataLine(QList<double>)));
    connect(logTailer,SIGNAL(headerRead(QString,int)),this,SLOT(addHeader(QString,int)));
    logTailer->readLine(fileName);
  }
}

void PlotDisplay::onProcessStopped(){
}


void PlotDisplay::clearPlot(){
  QList<int> ids = d_curves.keys();
  for(int i = 0;i<ids.size();i++){
    removeCurve(ids.at(i));
  }
}

void PlotDisplay::removeCurve(DatasetId id){
  QwtPlotCurve * curve = curveById(id);
  if(curve){
    curve->detach();
    delete curve;
    d_curves.remove(id);
  }
  CurveData * data = datasetById(id);
  if(data){
    delete data;
    d_datasets.remove(id);
  }  
}
