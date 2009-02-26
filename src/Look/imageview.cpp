#include <QtGui>
#include "imageview.h"

ImageView::ImageView(QWidget *parent)
{
  originalQi = NULL;
  //imageQi = NULL;
  leftPressed = false;
  leftPressed = false;
  paneActive = false;
  zoomActive = false;
  zoomValue = 1;
  zoomCenterX = 0.5; zoomCenterY = 0.5;
  centerX = 0.5; centerY = 0.5;
  moveCenterX = false;
  moveCenterY = false;
  defineBeamstopActive = false;
  beamstopMove = false;
  beamstopMoveR = false;
  beamstopX = 0.5;
  beamstopY = 0.5;
  beamstopR = 0.05;
  distanceBarLength = 0.0;
  circleResolution = 0.0;
  cursorCross = QCursor(Qt::CrossCursor);
  //pickSpotActive = false;
  /*
  imageLabel = new QLabel;
  imageLabel->setMinimumWidth(500);
  imageLabel->setMinimumHeight(500);
  imageLabel->setMaximumWidth(500);
  imageLabel->setMaximumHeight(500);
  */
  QGridLayout *layout = new QGridLayout;
  //layout->addWidget(imageLabel, 0, 0);
  setLayout(layout);
  setMouseTracking(true);
}

void ImageView::setImage(QImage *image)
{
  originalQi = image;
  //qi = qi.scaled(imageLabel->width(),imageLabel->height(),Qt::KeepAspectRatio);
  drawImage();
}

void ImageView::drawImage()
{
  if (originalQi == NULL) return;
  /*
  imageQi = originalQi->copy(QRect((int)((zoomCenterX-0.5*zoomValue)*originalQi->width()),(int)((zoomCenterY-0.5*zoomValue)*originalQi->height()),
				   (int)(zoomValue*originalQi->width()),(int)(zoomValue*originalQi->height()))).scaled(width(),height(),Qt::KeepAspectRatio);
  */

  imageQi = originalQi->copy(QRect((int)((zoomCenterX-0.5*zoomValue*(double)width()/(double)(width()<height()?width():height()))*originalQi->width()+0.5),
				   (int)((zoomCenterY-0.5*zoomValue*(double)height()/(double)(width()<height()?width():height()))*originalQi->height()+0.5),
				   (int)(zoomValue*originalQi->width()*(double)width()/(double)(width()<height()?width():height())+0.5),
				   (int)(zoomValue*originalQi->height()*(double)height()/(double)(width()<height()?width():height())+0.5))).
    scaled(width(),height(),Qt::KeepAspectRatio);

  //imageLabel->setPixmap(QPixmap::fromImage(imageQi));
  update();
}

int ImageView::globalToScreenX(double x)
{
  return (int) (((x - zoomCenterX) / zoomValue + 0.5) * (double) (width()<height() ? width():height()) + 0.5 +
		(double) (width() - (width()<height() ? width():height()))/2.0 );
}

int ImageView::globalToScreenY(double y)
{
  return (int) (((y - zoomCenterY) / zoomValue + 0.5) * (double) (width()<height() ? width():height()) + 0.5 +
		(double) (height() - (width()<height() ? width():height()))/2.0 );
}

// not tested yet
double ImageView::screenToGlobalX(int x)
{
  return zoomCenterX + (double) (2*x - width()) / 2.0 / (double) (width()<height() ? width():height()) * zoomValue;
  //return ((double) x / (double) width() - 0.5) * zoomValue * (double) (width()<height() ? width():height()) / (double) width() + zoomCenterX;
}

// not tested yet
double ImageView::screenToGlobalY(int y)
{
  return zoomCenterY + (double) (2*y - height()) / 2.0 / (double) (width()<height() ? width():height()) * zoomValue;
  //return ((double) y / (double) height() - 0.5) * zoomValue * (double) (width()<height() ? width():height()) / (double) height() + zoomCenterY;
}

bool ImageView::collumnToMask()
{
  return false;
}

void ImageView::setCenter(int on)
{
  if (on) setCenterActive = true;
  else setCenterActive = false;
  update();
}

bool ImageView::setCenterActivated()
{
  return setCenterActive;
}

void ImageView::setCenter(double x, double y)
{
  centerX = x; centerY = y;
  if (setCenterActive) update();
}

void ImageView::getCenter(double *x, double *y)
{
  *x = centerX; *y = centerY;
}

void ImageView::defineBeamstop(int on)
{
  if (on) defineBeamstopActive = true;
  else defineBeamstopActive = false;
  update();
}

bool ImageView::defineBeamstopActivated()
{
  return defineBeamstopActive;
}

void ImageView::setBeamstop(double x, double y, double r)
{
  beamstopX = x; beamstopY = y; beamstopR = r;
  if (defineBeamstopActive) update();
}

void ImageView::getBeamstop(double *x, double *y, double *r)
{
  *x = beamstopX; *y = beamstopY; *r = beamstopR;
}

void ImageView::showDistance(int on, bool bar, double value)
{
  if (on) {
    if (bar) {

      distanceBarLength = value;
      circleResolution = 0.0;
    } else {
      circleResolution = value;
      distanceBarLength = 0.0;
    }
  } else {
    distanceBarLength = 0.0;
    circleResolution = 0.0;
  }
  update();
}

void ImageView::getVertLine()
{
  pickSpotActive = true;
  QApplication::setOverrideCursor(cursorCross);
}

void ImageView::drawMask(bool on)
{
  undrawMaskActive = false;
  drawMaskActive = on;
}

void ImageView::undrawMask(bool on)
{
  drawMaskActive = false;
  undrawMaskActive = on;
}

void ImageView::mousePressEvent(QMouseEvent *event)
{
  if (pickSpotActive) {
    if (event->button() == Qt::LeftButton) {
      emit vertLineSet(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().x()));
      pickSpotActive = false;
      QApplication::restoreOverrideCursor();
      return;
    } else {
      pickSpotActive = false;
      QApplication::restoreOverrideCursor();
    }
  }
  if (leftPressed || rightPressed) return;
  if (event->button() == Qt::LeftButton) {
    if (drawMaskActive) {
      emit drawMaskAt(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().y()));
      leftPressed = true;
      return;
    }
    if (undrawMaskActive) {
      emit undrawMaskAt(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().y()));
      leftPressed = true;
      return;
    }
    if (setCenterActive) {
      int x = (int) (((centerX - zoomCenterX) / zoomValue + 0.5) * (double) width());  //(Cx - 0.5 - (zCx - 0.5) /zV = 0.5
      int y = (int) (((centerY - zoomCenterY) / zoomValue + 0.5) * (double) height());  //(Cy - 0.5 - (zCy - 0.5) /zV = 0.5
      if (event->pos().x() < x + (int)(0.05*(double)width()) &&
	  event->pos().x() > x - (int)(0.05*(double)width())) {
	moveCenterX = true;
	leftPressed = true;
      }
      if (event->pos().y() < y + (int)(0.05*(double)height()) &&
	  event->pos().y() > y - (int)(0.05*(double)height())){
	moveCenterY = true;
	leftPressed = true;
      }
    }
    if (defineBeamstopActive && !leftPressed) {
      int x = (int) (((beamstopX - zoomCenterX) / zoomValue + 0.5) * (double) width() + 0.5);
      int y = (int) (((beamstopY - zoomCenterY) / zoomValue + 0.5) * (double) height() + 0.5);
      int r = (int) (beamstopR / zoomValue * 0.8 * (double) (width()<height() ? width():height()) + 0.5);
      if ((event->pos().x()-x) * (event->pos().x()-x) + (event->pos().y()-y) * (event->pos().y()-y) < r*r) {
	beamstopMove = true;
	beamstopOffsetX = event->pos().x() - x;
	beamstopOffsetY = event->pos().y() - y;
	leftPressed = true;
      } else if ((event->pos().x()-x) *(event->pos().x()-x) + (event->pos().y()-y) * (event->pos().y()-y) < r*r*1.5*1.5) {
	beamstopMoveR = true;
	beamstopStartR = sqrt((event->pos().x()-x) *(event->pos().x()-x) + (event->pos().y()-y) * (event->pos().y()-y));
	beamstopOriginalR = beamstopR;
	leftPressed = true;
      }
    }
    if (!leftPressed) {
      paneActive = true;
      startPos = event->pos();
      startZoomCenterX = zoomCenterX;
      startZoomCenterY = zoomCenterY;
      leftPressed = true;
    }
  } else if (event->button() == Qt::RightButton) {
    if (!paneActive) {
      zoomActive = true;
      startPos = event->pos();
      startZoomValue = zoomValue;
      rightPressed = true;
    }
  } else {
    return;
  }
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
  if (!leftPressed && !rightPressed){
    emit mouseOverImage(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().y()));
    return;
  }
  if (paneActive) {
    //zoomCenterX = startZoomCenterX - (double)(event->pos().x() - startPos.x()) / (double) imageLabel->width() * zoomValue;
    //zoomCenterY = startZoomCenterY - (double)(event->pos().y() - startPos.y()) / (double) imageLabel->height() * zoomValue;
    zoomCenterX = startZoomCenterX - (double)(event->pos().x() - startPos.x()) / (double) width() * zoomValue;
    zoomCenterY = startZoomCenterY - (double)(event->pos().y() - startPos.y()) / (double) height() * zoomValue;
    if (zoomCenterX <  0.5*zoomValue) zoomCenterX = 0.5*zoomValue;
    if (zoomCenterX > 1.0 - 0.5*zoomValue) zoomCenterX = 1.0 - 0.5*zoomValue;
    if (zoomCenterY < 0.5*zoomValue) zoomCenterY = 0.5*zoomValue;
    if (zoomCenterY > 1.0 - 0.5*zoomValue) zoomCenterY = 1.0 - 0.5*zoomValue;
    drawImage();
  } else if (zoomActive) {
    zoomValue = startZoomValue + (double)(event->pos().y() - startPos.y()) / (double) height();
    //zoomCenterX = ((double) startPos.x()/ (double) imageLabel->width() - 0.5)*startZoomValue + startZoomCenterX + 0.5;
    //zoomCenterY = ((double) startPos.y()/ (double) imageLabel->height() - 0.5)*startZoomValue + startZoomCenterY + 0.5;
    if (zoomValue > 1) zoomValue = 1;
    if (zoomValue < 0.01) zoomValue = 0.01;
    if (zoomCenterX <  0.5*zoomValue) zoomCenterX = 0.5*zoomValue;
    if (zoomCenterX > 1.0 - 0.5*zoomValue) zoomCenterX = 1.0 - 0.5*zoomValue;
    if (zoomCenterY < 0.5*zoomValue) zoomCenterY = 0.5*zoomValue;
    if (zoomCenterY > 1.0 - 0.5*zoomValue) zoomCenterY = 1.0 - 0.5*zoomValue;
    drawImage();
  } else if (moveCenterX || moveCenterY) {
    if (moveCenterX) centerX = screenToGlobalX(event->pos().x()); //(event->pos().x() / (double) width() - 0.5) * zoomValue + zoomCenterX;
    if (moveCenterY) centerY = screenToGlobalY(event->pos().y()); //(event->pos().y() / (double) height() - 0.5) * zoomValue + zoomCenterY;
    update();
  } else if (beamstopMove) {
    beamstopX = screenToGlobalX(event->pos().x() - (int)beamstopOffsetX); 
    //((event->pos().x() - beamstopOffsetX) / (double) width() - 0.5) * zoomValue + zoomCenterX;
    beamstopY = screenToGlobalY(event->pos().y() - (int)beamstopOffsetY);
    //((event->pos().y() - beamstopOffsetY) / (double) height() - 0.5) * zoomValue + zoomCenterY;
    update();
  } else if (beamstopMoveR) {
    beamstopR = beamstopOriginalR * sqrt(((double)event->pos().x() - ((beamstopX - zoomCenterX) / zoomValue + 0.5) * (double) width()) *
					 ((double)event->pos().x() - ((beamstopX - zoomCenterX) / zoomValue + 0.5) * (double) width()) + 
					 ((double)event->pos().y() - ((beamstopY - zoomCenterY) / zoomValue + 0.5) * (double) height()) *
					 ((double)event->pos().y() - ((beamstopY - zoomCenterY) / zoomValue + 0.5) * (double) height()))
      / beamstopStartR;
    //int x = (int) (((beamstopX - zoomCenterX) / zoomValue + 0.5) * (double) width() + 0.5);
    update();
  } else if (drawMaskActive && leftPressed) {
    emit drawMaskAt(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().y()));
  } else if (undrawMaskActive && leftPressed) {
    emit undrawMaskAt(screenToGlobalX(event->pos().x()),screenToGlobalY(event->pos().y()));
  }
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
  
  if (event->button() == Qt::LeftButton) {
    if (moveCenterX || moveCenterY) emit centerChanged();
    if (beamstopMove || beamstopMoveR) emit beamstopChanged();
    moveCenterX = false;
    moveCenterY = false;
    beamstopMove = false;
    beamstopMoveR = false;
    paneActive = false;
    leftPressed = false;
  } else if (event->button() == Qt::RightButton) {
    zoomActive = false;
    rightPressed = false;
  }
}

void ImageView::leaveEvent(QEvent *event){
  zoomActive = false;
  rightPressed = false;
  emit mouseLeftImage();
}

void ImageView::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.drawImage(0,0,imageQi);
  if (setCenterActive) {
    painter.setPen(Qt::yellow);
    /*
    double x = (centerX - zoomCenterX) / zoomValue + 0.5;  //(Cx - 0.5 - (zCx - 0.5) /zV = 0.5
    double y = (centerY - zoomCenterY) / zoomValue + 0.5;  //(Cy - 0.5 - (zCy - 0.5) /zV = 0.5
    painter.drawLine(0,(int)((double)height()*y),width(),(int)((double)height()*y));
    painter.drawLine((int)((double)width()*x),0,(int)((double)width()*x),height());
    */
    painter.drawLine(0, globalToScreenY(centerY), width(), globalToScreenY(centerY));
    painter.drawLine(globalToScreenX(centerX), 0, globalToScreenX(centerX), height());
  }
  if (defineBeamstopActive) {
    painter.setPen(QPen(Qt::darkRed,3));
    //painter.pen().setWidth(3);
    //painter.setBrush(Qt::darkRed);
    int x = globalToScreenX(beamstopX); //(int)((double) width() * ((beamstopX - zoomCenterX) / zoomValue + 0.5));
    int y = globalToScreenY(beamstopY); //(int)((double) height() * ((beamstopY - zoomCenterY) / zoomValue + 0.5));
    int R = (int)(beamstopR / zoomValue * (double) (width()<height() ? width():height()) + 0.5);
    painter.drawEllipse(x - R, y - R, 2*R, 2*R);
  }
  if (circleResolution > 0.0) {
    painter.setPen(QPen(Qt::red,1));
    for (double n = 1.0; n <= 8.0; n *= 2.0) {
      painter.drawEllipse(globalToScreenX(centerX-0.5/n),globalToScreenY(centerY-0.5/n),
			  (int)((double) (width()<height()?width():height()) / n / zoomValue + 0.5),
			  (int)((double) (width()<height()?width():height()) / n / zoomValue + 0.5));
      QString label = QString::number(circleResolution * n);
      label.append(" nm");
      if (globalToScreenX(centerX + 0.3 / n) < width() && globalToScreenY(centerY - 0.3 / n) > 0)
	painter.drawText(QPoint(globalToScreenX(centerX + 0.3 / n),globalToScreenY(centerY - 0.3 / n)),label);
      else
	painter.drawText(QPoint(globalToScreenX(centerX - 0.3 / n - 0.1),globalToScreenY(centerY + 0.3 / n)),label);
    }
  }
  if (distanceBarLength > 0.0) {
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    double tmp1 = pow(10.0,((int) (log(0.3 * zoomValue * (double) width() * distanceBarLength)/log(10))));
    double tmp2 = 0.5 * pow(10.0,((int) (log(0.3 * zoomValue * (double) width() * distanceBarLength * 2.0)/log(10))));
    if (fabs(tmp2 - 0.3 * zoomValue * (double) width() * distanceBarLength) <
	fabs(tmp1 - 0.3 * zoomValue * (double) width() * distanceBarLength))
      tmp1 = tmp2;
    painter.drawRect((int)(width() * 0.9 - tmp1/distanceBarLength/zoomValue + 0.5),(int)(height() * 0.9),
		     (int) (tmp1/distanceBarLength/zoomValue + 0.5), (int)(height() * 0.02));
    QString label = QString::number(tmp1);
    label.append(" nm");
    painter.setPen(Qt::red);
    painter.drawText(QPoint((int)((double) width() * 0.8), (int)((double)height() * 0.89)), label);
  }
}
    
void ImageView::resizeEvent(QResizeEvent *event)
{
  drawImage();
  update();
}
