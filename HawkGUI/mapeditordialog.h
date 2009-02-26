#ifndef _MAP_EDITOR_DIALOG_H_
#define _MAP_EDITOR_DIALOG_H_ 1


#include <QDialog>
#include <QPen>
#include <QBrush>

class QLineEdit;
class QDialogButtonBox;

class MapEditorPopup : public QWidget
{
  Q_OBJECT
    public:
  MapEditorPopup(QPoint pos,QPointF value, int index,QWidget * parent );
  int index;
  QLineEdit * xEdit;
  QLineEdit * yEdit;

 signals:
  void editingFinished();
 protected:
  void keyPressEvent(QKeyEvent * event);
};

class MapEditorWidget : public QWidget
{
  Q_OBJECT    
public:
  MapEditorWidget(QWidget * parent = 0);
  void firePointChange();
  void setPoints(const QPolygonF & new_points);
  void setXLabel(QString label);
  void setYLabel(QString label);
  QPolygonF getPoints(){
    return points;
  }
signals:
    void pointsChanged(const QPolygonF &points);
 public slots:
   void updatePointFromPopup();
    
 protected:
  void mousePressEvent ( QMouseEvent * event );
  void paintEvent ( QPaintEvent * event );
  void mouseMoveEvent(QMouseEvent * event);
  void mouseReleaseEvent ( QMouseEvent * event );
  void mouseDoubleClickEvent(QMouseEvent * event);
 private:
  void movePoint(int index, const QPointF &point, bool emitUpdate = true);
  void paintPoints();
  void paintAxis();
  inline QRectF pointBoundingRect(int i) const;
  inline QRectF boundingRect() const;
  QPointF boundPoint(const QPointF &point, const QRectF &bounds);
  QRectF scaledPlotArea();
  void updateTransform();
  QWidget * plot;
  QPolygonF points;
  QRectF bounds;
  int currentIndex;
  QSizeF pointSize;
  QMatrix transform;
  QPen pointPen;
  QBrush pointBrush;
  QPen connectionPen;
  QRectF plotArea;
  bool popupEditorShown;
  MapEditorPopup * popup;
  QString xLabel;
  QString yLabel;
};

class MapEditorDialog : public QDialog
{
  Q_OBJECT    
public:
  MapEditorDialog(QWidget * parent = 0);
  QDialogButtonBox * buttonBox;
  void setPoints(const QPolygonF & new_points){
    editor->setPoints(new_points);
  }
  void setXLabel(QString s){
    editor->setXLabel(s);
  }
  void setYLabel(QString s){
    editor->setYLabel(s);
  }
  QPolygonF getPoints(){
    return editor->getPoints();
  }
 public slots:
  void accept();
  
 private:
  MapEditorWidget * editor;
};


inline QRectF MapEditorWidget::pointBoundingRect(int i) const
{
  QPointF p = transform.map(points.at(i));
  double w = pointSize.width();
  double h = pointSize.height();
  double x = p.x() - w / 2;
  double y = p.y() - h / 2;
  return QRectF(x, y, w, h);
}

inline QRectF MapEditorWidget::boundingRect() const
{
    if (bounds.isEmpty())
        return rect();
    else
        return bounds;
}

#endif
