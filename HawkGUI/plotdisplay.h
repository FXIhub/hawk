#ifndef _PLOTDISPLAY_H_
#define _PLOTDISPLAY_H_ 1

#include <qwt_array.h>
#include <qwt_plot.h>
#include <QTimer>
#include <QMap>

class QwtPlotCurve;
class Zoomer;
class ProcessControl;
class LogTailer;

class CurveData
{
 public:

    CurveData();

    void append(double *x, double *y, int count);

    int count() const;
    int size() const;
    const double *x() const;
    const double *y() const;
private:
    int d_count;
    QwtArray<double> d_x;
    QwtArray<double> d_y;
};


class PlotDisplay : public QwtPlot
{
  Q_OBJECT
    public:
  PlotDisplay(QWidget  * parent);
  virtual ~PlotDisplay();
  typedef int DatasetId;
  void createDataset(QString name, int id);
  int appendData(DatasetId id,double x, double y);
  int appendData(DatasetId id,double *x, double *y, int size);  

  public slots:
  void onProcessStarted(QString type, QString path,ProcessControl * p);
  void onProcessStopped();
  void loadUserSelectedLogFile();
  private slots:
  int setCurveVisible(DatasetId id,bool visible);
  int setCurveVisible(QwtPlotItem * plotItem,bool visible);
  void addDataLine(QList<double> data);
  void addHeader(QString title, int col);
  void updateCurves();
 private:
  void updateZoomBase();
  CurveData * datasetById(DatasetId id);
  QwtPlotCurve * curveById(DatasetId id);
  QVector<QColor> generatePlotColors(int n);
  void removeCurve(DatasetId id);
  void clearPlot();
  QMap<int,CurveData *> d_datasets;
  QMap<int, QwtPlotCurve *> d_curves;
  Zoomer * zoomer;
  LogTailer * logTailer;
  bool dirty;
  QTimer updateTimer;
};

#endif
