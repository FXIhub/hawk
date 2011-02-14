#include <QtGui>
#include "stitcherview.h"
#include "imageitem.h"
#include "imageviewpanel.h"

StitcherView::StitcherView(QWidget * parent)
  :ImageView(parent)
{
  imageViewPanel()->showSaveButton(true);
  setRenderHints(QPainter::Antialiasing);
  /*  QGraphicsLineItem * centerVerticalIndicator = new QGraphicsLineItem(0,-100000,0,100000);
  QGraphicsLineItem * centerHorizontalIndicator = new QGraphicsLineItem(-100000,0,100000,0);
  centerVerticalIndicator->setZValue(11);
  centerHorizontalIndicator->setZValue(11);
  graphicsScene->addItem(centerVerticalIndicator);
  graphicsScene->addItem(centerHorizontalIndicator);
  QPen pen = centerHorizontalIndicator->pen();  
  pen.setColor(Qt::white);
  pen.setStyle(Qt::SolidLine);
  centerHorizontalIndicator->setPen(pen);
  centerVerticalIndicator->setPen(pen);*/

  /* Solid background */
  setBackgroundBrush(QColor("#26466D"));
  _showIdentifiers = true;
  setBackgroundDraggable(false);
}

bool StitcherView::loadImage(QString s){
  Image * a =  sp_image_read(s.toAscii(),0);
  ImageItem * item = new ImageItem(a,s,this,NULL);
  addImage(item);    
  item->update();
  return true;
}


void StitcherView::addImage(ImageItem * item){
  int display = -1;
  int color = -1;
  bool isShifted = false;
  setSelectedImage(item);
  //  item->setPos(-item->pixmap().width()/2,-item->pixmap().height()/2);
  /* Always add in the same position */
  item->setPos(0,0);
  /* translate so that the item origin corresponds to the image center */
  item->translate(-item->pixmap().width()/2,-item->pixmap().height()/2);
  if(preservesShift() && isShifted != item->isShifted()){
    item->shiftImage();
  }
  if(color >= 0){
    item->setColormap(color);
  }
  if(display >= 0){
    item->setDisplay(display);
  }
  graphicsScene->addItem(item);  
  item->showIdentifier(_showIdentifiers);
  emit imageItemChanged(item);
  emit imageItemGeometryChanged(item);
}


void StitcherView::clearAll(){
  scene()->clear();
  constraintFit.clear();
  _selected = NULL;
  emit imageItemGeometryChanged(NULL);
}

void StitcherView::mouseReleaseEvent( QMouseEvent *  event){
  ImageView::mouseReleaseEvent(event);
  if(mode == Line && event->button() & Qt::LeftButton){
    setMode(Default);
    QGraphicsLineItem * line = new QGraphicsLineItem(QLineF(mapToScene(lineOrigin),mapToScene(lineEnd)));
    line->setData(0,QString("Helper"));
    line->setZValue(11);    
    QPen pen = line->pen();  
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    line->setPen(pen);
    graphicsScene->addItem(line);    
  }else if(mode == Circle && event->button() & Qt::LeftButton){
    setMode(Default);
    QPointF lineOriginF = mapToScene(lineOrigin);
    QPointF lineEndF = mapToScene(lineEnd);
    QPointF circleCenter = (lineOriginF+lineEndF)/2;    
    qreal circleRadius = sqrt((lineOriginF-lineEndF).x()* (lineOriginF-lineEndF).x()+
			      (lineOriginF-lineEndF).y()* (lineOriginF-lineEndF).y())/2;
    if(QApplication::keyboardModifiers() & Qt::ShiftModifier){
      circleCenter =  mapFromScene(QPointF(0,0));
      circleRadius = sqrt((circleCenter-lineEnd).x()* (circleCenter-lineEnd).x()+
			      (circleCenter-lineEnd).y()* (circleCenter-lineEnd).y());
      circleCenter =  QPointF(0,0);

    }
    QGraphicsEllipseItem * circle = new QGraphicsEllipseItem(QRect(circleCenter.x()-circleRadius,circleCenter.y()-circleRadius,circleRadius*2,circleRadius*2));
    circle->setData(0,QString("Helper"));
    circle->setZValue(11);    
    QPen pen = circle->pen();  
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    circle->setPen(pen);
    graphicsScene->addItem(circle);    
  }else if(mode == AddPoint && event->button() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it.at(i))){
	item->addControlPoint(item->mapFromScene(mapToScene(event->pos())));
      }
    }
  }else if(mode == DeletePoint && event->button() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->pos());
    for(int i = 0; i < it.size(); i++){
      if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it.at(i))){
	item->deleteControlPoint(item->mapFromScene(mapToScene(event->pos())));
      }
    }
  }else if(mode == DeleteGuide && event->button() & Qt::LeftButton){
    QList<QGraphicsItem *> it = items(event->pos());
    QPointF pos = mapToScene(event->pos());
    /* 10 px tolerance radius, delete the closest */
    for(int i = 0; i < it.size(); i++){
      if(QString("Helper") == it[i]->data(0)){
	QGraphicsEllipseItem * elipse = qgraphicsitem_cast<QGraphicsEllipseItem *>(it[i]);
	if(elipse){
	  // Check if click position close to the line
	  QPointF origin = elipse->rect().center();
	  qreal radius = elipse->rect().height()/2;
	  QPointF d = origin-pos;	  
	  if(abs(sqrt(d.x()*d.x()+d.y()*d.y())-radius) < 10){
	    delete elipse;
	  }
	}
	QGraphicsLineItem * line = qgraphicsitem_cast<QGraphicsLineItem *>(it[i]);
	if(line){
	  
	  // Check if click position close to the line
	  QPointF origin = elipse->rect().center();
	  qreal radius = elipse->rect().height()/2;
	  QPointF d = origin-pos;	  
	  if(abs(sqrt(d.x()*d.x()+d.y()*d.y())-radius) < 10){
	    delete elipse;
	  }
	}

      }
    }
  }
}



void StitcherView::saveImage(){
  if(selectedImage() && selectedImage()->getImage()){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"));
    qDebug("Trying to save %s",fileName.toAscii().data());
    if(!fileName.isEmpty()){
      sp_image_write(selectedImage()->getImage(),fileName.toAscii().data(),0);
    }
  }
}

void StitcherView::setMode(Mode m){
  mode = m;
  if(m == Line || m == Circle || m == AddPoint || m == DeletePoint || m == DeleteGuide){
    setCursor(Qt::CrossCursor);
  }
  if(m == Default){
    setCursor(Qt::ArrowCursor);
  }
}

void StitcherView::paintEvent(QPaintEvent *event){
  ImageView::paintEvent(event);
  QPainter p(viewport());
  if(mode == Line && QApplication::mouseButtons() & Qt::LeftButton){
    /* paint line out */
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawLine(lineOrigin,lineEnd);
  }else if(mode == Circle && QApplication::mouseButtons() & Qt::LeftButton){
    QPointF circleCenter = (lineOrigin+lineEnd)/2;
    qreal circleRadius = sqrt((lineOrigin-lineEnd).x()* (lineOrigin-lineEnd).x()+
			      (lineOrigin-lineEnd).y()* (lineOrigin-lineEnd).y())/2;
    if(QApplication::keyboardModifiers() & Qt::ShiftModifier){
      circleCenter =  mapFromScene(QPointF(0,0));
      circleRadius = sqrt((circleCenter-lineEnd).x()* (circleCenter-lineEnd).x()+
			      (circleCenter-lineEnd).y()* (circleCenter-lineEnd).y());

    }
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    p.drawEllipse(QRectF(circleCenter.x()-circleRadius,circleCenter.y()-circleRadius,circleRadius*2,circleRadius*2));
  }  

}

void StitcherView::mousePressEvent( QMouseEvent *  event){
  if(mode == Line || mode == Circle){
    lineOrigin = event->pos();
  }else{
    if(event->button() & Qt::LeftButton){
      QList<QGraphicsItem *> it = items(event->pos());
      for(int i = 0; i < it.size(); i++){
	if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it[i])){
	  if(!item->isObscured(QRectF(item->mapFromScene(mapToScene(event->pos())),QSize(1,1)))){
	    setSelectedImage(item);
	    break;
	  }
	}
      }
    }
    ImageView::mousePressEvent(event);
  }
}

void StitcherView::mouseMoveEvent(QMouseEvent * event){
  if(mode == Line || mode == Circle){
    lineEnd = event->pos();
    scene()->update();
  } else if(dragged && event->buttons() & Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier)){
    QPointF mov = dragged->mapFromScene(mapToScene(event->pos()))-dragged->mapFromScene(mouseLastScenePos);
    dragged->setDx(dragged->dx()+mov.x());
    dragged->setDy(dragged->dy()+mov.y());
    emit imageItemGeometryChanged(dragged);
  }else if(event->buttons() & Qt::LeftButton){
    QPointF mov = mapToScene(event->pos())-mouseLastScenePos;
    //    emit translateBy(mov);
    setSceneRect(sceneRect().translated(-mov));
  }else if(event->buttons() & Qt::RightButton){
    if(event->modifiers() & Qt::ShiftModifier){
      /* Only change Dz of selected image */
      if(selectedImage()){
	QPointF mouse_mov = mapToScene(event->pos())-mouseLastScenePos;
	qreal speed = 0.005;
	qreal scale = 1-mouse_mov.y()*speed;
	selectedImage()->setDz(selectedImage()->dz()*scale);
	emit imageItemGeometryChanged(selectedImage());
      }
    }else{
      QPointF mouse_mov = mapToScene(event->pos())-mouseLastScenePos;
      qreal speed = 0.005;
      qreal scale = 1-mouse_mov.y()*speed;
      scaleScene(scale);
    }
  }
  mouseOverValue(event);
  mouseLastScenePos = mapToScene(event->pos());
  event->accept();
}

void StitcherView::wheelEvent( QWheelEvent * event ){
  qreal speed = 0.0005;
  qreal scale = 1+event->delta()*speed;
  scaleScene(scale);
}

void StitcherView::keyPressEvent ( QKeyEvent * event ){
  ImageView::keyPressEvent(event);
}


void StitcherView::scaleScene(qreal new_scale){
  if(new_scale * transform().m11() > 0.01 && new_scale * transform().m11() < 100){
    scale(new_scale,new_scale);
  }
}

void StitcherView::scaleItems(qreal new_scale){
  QList<QGraphicsItem *> it = items();
  for(int i = 0; i < it.size(); i++){
    if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it[i])){
      item->setDz(item->dz()*new_scale);
      emit imageItemGeometryChanged(item);
    }
  }    
}



void StitcherView::clearConstraintFits(){
  while(!constraintFit.isEmpty()){
    delete constraintFit.takeLast();
  }
}

void StitcherView::drawConstraintFit(geometrically_constrained_system *gc){
  for(int i = 0;i<gc->n_constraints;i++){
    GeometryConstraintType type = gc->constraints[i].type;
    double fit = gc->constraints[i].best_fit;    
    if(type == RadialLineConstraint){
      qreal x1,x2,y1,y2;
      qreal scale = 10000;
      x1 = cos(fit)*scale;

      y1 = sin(fit)*scale;
      x2 = -x1;
      y2 = -y1;
      QGraphicsLineItem * item = new QGraphicsLineItem(x1,y1,x2,y2);
      QPen p = item->pen();
      p.setStyle(Qt::DashLine);
      QVector<qreal> dashes;
      dashes << 25 << 15;
      p.setDashPattern(dashes);
      p.setColor(Qt::white);
      item->setPen(p);
      item->setZValue(10000);
      scene()->addItem(item);
      constraintFit.append(item);
    }else if(type == CircleConstraint){
      double radius = fit;
      QGraphicsEllipseItem * item = new QGraphicsEllipseItem(-radius,-radius,2*radius,2*radius);
      QPen p = item->pen();
      p.setStyle(Qt::DashLine);
      QVector<qreal> dashes;
      dashes << 25 << 15;
      p.setDashPattern(dashes);
      p.setColor(Qt::white);
      item->setPen(p);
      item->setZValue(10000);
      scene()->addItem(item);
      constraintFit.append(item);
    }
    sp_vector ** cp_g = control_point_list_to_global(gc->constraints[i].points,gc->constraints[i].n_points);
    QColor color = QColor::fromHsvF(1.0/3+(double)i/gc->n_constraints,1,1,1);
    for(int j = 0;j<gc->constraints[i].n_points;j++){
      QGraphicsEllipseItem * point = new QGraphicsEllipseItem(-4,-4,8,8);
      point->setZValue(10001);
      point->setPos(sp_vector_get(cp_g[j],0),sp_vector_get(cp_g[j],1));
      point->setPen(QPen(color, 2));
      point->setFlags(point->flags() | QGraphicsItem::ItemIgnoresTransformations);
      scene()->addItem(point);
      constraintFit.append(point);
    }
  }
}
