#include <math.h>
#include <QtGui>
#include <float.h>
#include "mapeditordialog.h"


inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}


MapEditorDialog::MapEditorDialog(QWidget * parent)
  :QDialog(parent)
{
  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
						      | QDialogButtonBox::Cancel,
  						      Qt::Horizontal,this);
  //  buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
  buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
  buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(true);
  //  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  //  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  QVBoxLayout * layout = new QVBoxLayout;
  editor = new MapEditorWidget(this);
  layout->addWidget(editor);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

void MapEditorDialog::accept(){
  /* this seems to be necessary as when the dialog is hidden the
     delegate immediatly commits the editor data to the model*/
  editor->finishEditing();
  setResult(Accepted);
  done(Accepted);
}



MapEditorWidget::MapEditorWidget(QWidget * parent)
  : QWidget(parent)
{
  
  int width = 700;
  int height = 350;
  plotArea = QRectF(70,15,width-85,height-35);
  pointSize = QSize(11, 11);
  currentIndex = -1;
  pointPen = QPen(QColor("#26466D"), 1);
  connectionPen = QPen(QColor("#B2DFEE"), 4);
  pointBrush = QBrush(QColor(0x1fb2dfee));
  setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
  setMinimumSize(width,height);
  popup = NULL;
  setToolTip("<p>Drag,left,right and double click to move,insert, delete"
	     " and change value of a point.</p>");
  connect(this, SIGNAL(pointsChanged(const QPolygonF &)),
	  this, SLOT(update()));  
}

void MapEditorWidget::mouseDoubleClickEvent ( QMouseEvent * event ){
  QPointF clickPos = transform.inverted().map(QPointF(event->pos()));
  int index = -1;
  if(popup){
    /* there is already a point being edited */
    return;
  }
  for (int i=0; i<points.size(); ++i) {
    QPainterPath path;
    path.addEllipse(pointBoundingRect(i));
    if (path.contains(event->pos())) {
      index = i;
      break;
    }
  }
  if(index != -1){
    popup = new MapEditorPopup(event->pos(),points[index],index,this);    
    popup->show();
    if(plotArea.contains(popup->geometry()) == false){
      QRectF united = plotArea.united(popup->geometry());
      QPoint move(0,0);
      if(united.bottomRight() != plotArea.bottomRight()){
	if(united.bottomRight().x()!=plotArea.bottomRight().x()){
	  move.rx() -= popup->width()+pointSize.width();
	}
	move.ry() -= united.bottomRight().y()-plotArea.bottomRight().y();
      } 
      if(united.topLeft() != plotArea.topLeft()){
	move.rx() -= united.topLeft().x()-plotArea.topLeft().x();
	move.ry() -= united.topLeft().y()-plotArea.topLeft().y();
      } 
      popup->move(popup->pos()+move);
    }
    connect(popup,SIGNAL(editingFinished()),this,SLOT(updatePointFromPopup()));
  }else{
    QWidget::mouseDoubleClickEvent(event);
  }
}

void MapEditorWidget::mousePressEvent ( QMouseEvent * event ){
  if(popup){
    return;
  }
  QPointF clickPos = transform.inverted().map(QPointF(event->pos()));
  int index = -1;
  for (int i=0; i<points.size(); ++i) {
    QPainterPath path;
    path.addEllipse(pointBoundingRect(i));
    if (path.contains(event->pos())) {
      index = i;
      break;
    }
  }
  if(plotArea.contains(event->pos()) == false && index == -1){
    return;
  }


  if (event->button() == Qt::LeftButton) {
    if (index == -1) {
      int pos = 0;
      // Insert sort for x or y
      for (int i=0; i<points.size(); ++i){
	if (points.at(i).x() > clickPos.x()) {
	  pos = i;
	  break;
	}
      }
      points.insert(pos, clickPos);
      currentIndex = pos;
      firePointChange();
    } else {
      currentIndex = index;
    }
  } else if (event->button() == Qt::RightButton) {
    if (index >= 0) {
      if(points.size() > 1){
	points.remove(index);
	firePointChange();
      }
    }
  }
}


void MapEditorWidget::mouseReleaseEvent ( QMouseEvent * ){
  currentIndex = -1;
  updateTransform();
  firePointChange();
}

void MapEditorWidget::mouseMoveEvent(QMouseEvent * event){
  if (currentIndex >= 0)
    movePoint(currentIndex, transform.inverted().map(QPointF(event->pos())));
}

void MapEditorWidget::firePointChange()
{  
  QPointF oldCurrent;
  if (currentIndex != -1) {
    oldCurrent = points[currentIndex];
  }
  
  qSort(points.begin(), points.end(), x_less_than);
  
  // Compensate for changed order...
  if (currentIndex != -1) {
    for (int i=0; i<points.size(); ++i) {
      if (points[i] == oldCurrent) {
	currentIndex = i;
	break;
      }
    }
  }
  //  update();
    emit pointsChanged(points);
}


QPointF MapEditorWidget::boundPoint(const QPointF &point, const QRectF &)
{
    QPointF p = point;
    if(p.x() < 0){
      p.setX(0);
    }
    /*
    double left = transform.inverted().map(bounds.topLeft()).x();
    double right = transform.inverted().map(bounds.bottomRight()).x();
    double top = transform.inverted().map(bounds.topLeft()).y();
    double bottom = transform.inverted().map(bounds.bottomRight()).y();

    if (p.x() < left){
      p.setX(left);
    }else if (p.x() > right){
      p.setX(right);
    }

    if (p.y() > top){
      p.setY(top);
    }else if (p.y() < bottom){
      p.setY(bottom);
    }
    */
    return p;
}

void MapEditorWidget::movePoint(int index, const QPointF &point, bool emitUpdate)
{
    points[index] = boundPoint(point, plotArea);
  if (emitUpdate){
    firePointChange();
  }
}

void MapEditorWidget::paintPoints()
{
  QPolygonF sPoints = transform.map(points);
  QPainter p;
  p.begin(this);
  
  p.setRenderHint(QPainter::Antialiasing);

  p.setPen(QColor("#000000"));
  p.setBrush(QBrush());
  //  p.drawRect(plotArea);
  
  p.setPen(connectionPen);
  if(sPoints.size()){
    QPainterPath path;
    path.moveTo(QPointF(plotArea.topLeft().x(),sPoints.at(0).y()));
    path.lineTo(QPointF(sPoints.at(0)));
      //    path.moveTo(sPoints.at(0));
    for (int i=1; i<sPoints.size(); ++i) {
      QPointF p1 = sPoints.at(i-1);
      QPointF p2 = sPoints.at(i);
      double distance = p2.x() - p1.x();
      
      path.cubicTo(p1.x() + distance / 2, p1.y(),
		   p1.x() + distance / 2, p2.y(),
		   p2.x(), p2.y());
    }
    path.lineTo(plotArea.bottomRight().x(),sPoints.last().y());
    p.drawPath(path);    
    path.lineTo(plotArea.bottomRight());
    path.lineTo(plotArea.topLeft().x(),plotArea.bottomRight().y());
    QLinearGradient grad = QLinearGradient(0,0,0,plotArea.height());
    grad.setColorAt(0,QColor("#B2DFEE"));
    grad.setColorAt(1,QColor("#26466D"));
    QBrush grad_brush(grad);      
    p.fillPath(path,grad_brush);
  }
  p.setPen(pointPen);
  p.setBrush(pointBrush);
  for (int i=0; i<sPoints.size(); ++i) {
    QRectF bounds = pointBoundingRect(i);
    p.drawEllipse(bounds);
  }



}

void MapEditorWidget::paintAxis(){
  QPainter p;
  p.begin(this);
  
  p.setRenderHint(QPainter::Antialiasing);

  p.setPen(QColor("#000000"));
  p.setBrush(QBrush());
  p.drawRect(plotArea);

  p.setPen(QColor("#000000"));
  
  QString maxY = QString::number(scaledPlotArea().bottomRight().y(),'g',4);
  QString minY = QString::number(scaledPlotArea().topLeft().y(),'g',4);
  QPointF pos = QPointF(plotArea.topLeft());
  QFontMetrics fm(p.font());
  pos.rx() -= fm.boundingRect(maxY).width()+5;
  pos.ry() += fm.boundingRect(maxY).height()/2;
  p.drawText(pos,maxY);

  pos = QPointF(plotArea.topLeft().x(),plotArea.bottomRight().y());
  pos.rx() -= fm.boundingRect(minY).width()+5;
  pos.ry() += fm.boundingRect(minY).height()/2-5;
  p.drawText(pos,minY);

  QString maxX = QString::number(scaledPlotArea().bottomRight().x(),'g',4);
  QString minX = QString::number(scaledPlotArea().topLeft().x(),'g',4);

  pos = QPointF(plotArea.topLeft().x(),plotArea.bottomRight().y());
  pos.rx() -= fm.boundingRect(minX).width()/2;
  pos.ry() += fm.boundingRect(minX).height();
  p.drawText(pos,minX);

  pos = QPointF(plotArea.bottomRight());
  pos.rx() -= fm.boundingRect(maxX).width();
  pos.ry() += fm.boundingRect(maxX).height();
  p.drawText(pos,maxX);

  if(!xLabel.isEmpty()){
    pos = QPointF(plotArea.bottomRight());
    pos.rx() -= fm.boundingRect(xLabel).width()/2+plotArea.width()/2;
    pos.ry() += fm.boundingRect(xLabel).height();
    p.drawText(pos,xLabel);
  }

  if(!yLabel.isEmpty()){
    pos = QPointF(-plotArea.topLeft().y(),plotArea.topLeft().x());
    p.rotate(-90);
    pos.ry() -= fm.boundingRect(yLabel).height()/2;
    pos.rx() -= fm.boundingRect(yLabel).width()/2+plotArea.height()/2;
    p.drawText(pos,yLabel);
  }

}

void MapEditorWidget::paintEvent(QPaintEvent * ){
  paintAxis();
  paintPoints();
}

QRectF MapEditorWidget::scaledPlotArea(){
  if(points.size() == 0){
    return QRectF(0,0,1,1);
  }
  if(points.size() == 1){
    QRectF ret = QRectF(points[0].x()-fabs(points[0].x()),points[0].y()-fabs(points[0].y()),fabs(points[0].x())*2,fabs(points[0].y())*2);
    if(points[0].x() == 0){
      ret.setWidth(1);
    }
    if(points[0].y() == 0){
      ret.setHeight(1);
    }
    return ret;
  }
  QPointF min = points[0];
  QPointF max = points[0];
  for (int i=1; i<points.size(); ++i) {
    if(points[i].x() < min.x()){
      min.setX(points[i].x());
    }
    if(points[i].y() < min.y()){
      min.setY(points[i].y());
    }
    if(points[i].x() > max.x()){
      max.setX(points[i].x());
    }
    if(points[i].y() > max.y()){
      max.setY(points[i].y());
    }
  }

  min.setX(0);
  if(min.x() == max.x()){
    max.rx() = 1;
  }

  if(min.y() == max.y()){
    if(min.y()){
      max.ry() += min.y();
      min.ry() = 0;
    }else{
      max.ry() = 1;
    }
  }
//  min *= 0.98;
  //  max *= 1.02;
  
  //  min.setX( copysign( pow(2, (int)logb( fabs(min.x()) ) ),min.x()));
  //  min.setY( copysign( pow(2, (int)logb( fabs(min.y()) ) ),min.y()));
  //  max.setX( copysign( pow(2, (int)logb( fabs(max.x()) )+1),max.x()));
  //  max.setY( copysign( pow(2, (int)logb( fabs(max.y()) )+1),max.y()));
  return QRectF(min,max);
}


void MapEditorWidget::setPoints(const QPolygonF &new_points)
{
    points.clear();
    for (int i=0; i<new_points.size(); ++i){
      points << new_points.at(i);
    }
    updateTransform();
}

void MapEditorWidget::updateTransform(){
  QRectF scaled = scaledPlotArea();
  QPointF trans;
  trans.setX((plotArea.topLeft()-scaled.topLeft()).x());
  trans.setY((plotArea.bottomRight()-scaled.topLeft()).y());
  QPointF scale;
  scale.setX((plotArea.bottomRight()-plotArea.topLeft()).x()/
	     (scaled.bottomRight()-scaled.topLeft()).x());

  scale.setY(-(plotArea.bottomRight()-plotArea.topLeft()).y()/
	     (scaled.bottomRight()-scaled.topLeft()).y());
  trans.setX(plotArea.bottomRight().x() - scale.x()*scaled.bottomRight().x());
  trans.setY(plotArea.bottomRight().y() - scale.y()*scaled.topLeft().y());
  transform.reset();
  transform.translate(trans.x(),trans.y());
  transform.scale(scale.x(),scale.y()); 
}

void MapEditorWidget::updatePointFromPopup(){
  //  MapEditorPopup * popup = qobject_cast<MapEditorPopup *>(sender());
  if(popup){
    QPointF p = QPointF(popup->xEdit->text().toDouble(), popup->yEdit->text().toDouble());
    movePoint(popup->index, p);
    updateTransform();
    firePointChange();
    popup->close();
    popup = NULL;
  }else{
    qDebug("Tried to update non existing popup");
  }
}

void MapEditorWidget::setXLabel(QString label){
  xLabel = label;
}

void MapEditorWidget::setYLabel(QString label){
  yLabel = label;
}

void MapEditorWidget::finishEditing(){
  if(popup){
    updatePointFromPopup();
  }
}

MapEditorPopup::MapEditorPopup(QPoint pos,QPointF value, int i,QWidget * parent)
    :QWidget(parent)
//  :QWidget(parent,Qt::Popup)
//  :QDialog(parent,Qt::Popup)
{
  index = i;
  pos.rx() += 5;
  pos.ry() -= 5;
  QGridLayout * layout = new QGridLayout;
  layout->addWidget(new QLabel("x:"),0,0);
  xEdit = new QLineEdit;
  xEdit->setText(QString::number(value.x(),'g',6));
  xEdit->setMaximumWidth(80);
  QDoubleValidator * sv = new QDoubleValidator(xEdit);
  sv->setNotation(QDoubleValidator::ScientificNotation);
  xEdit->setValidator(sv);

  layout->addWidget(xEdit,0,1);
  layout->addWidget(new QLabel("y:"),1,0);
  yEdit = new QLineEdit;
  yEdit->setText(QString::number(value.y(),'g',6));
  yEdit->setMaximumWidth(80);
  sv = new QDoubleValidator(yEdit);
  sv->setNotation(QDoubleValidator::ScientificNotation);
  yEdit->setValidator(sv);

  layout->addWidget(yEdit,1,1);
  setLayout(layout);
  pos.ry() -= sizeHint().height()/2;
  //  move(parent->mapToGlobal(pos));
  move(pos);
  xEdit->setFocus(Qt::ActiveWindowFocusReason);
  //  setAttribute(Qt::WA_DeleteOnClose,true);
}

void MapEditorPopup::keyPressEvent(QKeyEvent * event){
  if(event->key() == Qt::Key_Return ||
     event->key() == Qt::Key_Enter){
    event->accept();
    emit editingFinished();
  }
}

