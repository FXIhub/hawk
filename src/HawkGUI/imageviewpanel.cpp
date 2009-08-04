#include "imageviewpanel.h"
#include "imageview.h"
#include "spimage.h"
#include <QtGui>

ImageViewPanel::ImageViewPanel(ImageView * parent)
  :QWidget(parent)
{
  imageView = parent;
  frame = NULL;
  installEventFilter(this);
  QVBoxLayout * vbox = new QVBoxLayout(this);
  setLayout(vbox);
  vbox->setContentsMargins(0,0,0,0);
  //   frame = new QFrame(this);
  frame = new QScrollArea(this);
  frame->setObjectName("panelFrame");
  vbox->addWidget(frame);
  //  vbox = new QVBoxLayout(frame);
  //  frame->setLayout(vbox);
  //  vbox->setContentsMargins(0,0,0,0);
  QToolBar * toolbar = new QToolBar(frame);
  toolbar->setObjectName("panelToolBar");
  frame->setWidget(toolbar);
    frame->setWidgetResizable(true);

  //  vbox->addWidget(toolbar);
  QWidget * stretcher = new QWidget(toolbar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  toolbar->addWidget(stretcher);
  displayCombo = new QComboBox;  
  displayCombo->addItem("Amplitudes",0);
  displayCombo->addItem("Phases",SpColormapPhase);
  displayCombo->addItem("Mask",SpColormapMask);
  connect(displayCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onDisplayComboChanged(int)));
  toolbar->addWidget(displayCombo);
  colormapCombo = new QComboBox(toolbar);
  colormapCombo->setToolTip(tr("Select Colormap"));
  connect(colormapCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(changeColormap(int)));
  colormapCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  QSize iconSize = QSize(22,22);
  colormapCombo->setIconSize(iconSize);
  for(int colormap =  SpColormapFirstColorScheme;colormap < SpColormapLastColorScheme;colormap*=2){
    QLinearGradient gradient = QLinearGradient(QPointF(0,0),QPointF(iconSize.width(),0));
    for(int step = 0;step < 10;step++){
      real value = step/10.0;
      sp_rgb rgb = sp_colormap_rgb_from_value(value,colormap);
      gradient.setColorAt(value,QColor(rgb.r,rgb.g,rgb.b));
    }
    QPixmap pixmap = QPixmap(iconSize);
    QPainter painter(&pixmap);
    painter.fillRect(0, 0, iconSize.width(), iconSize.height(), gradient);
    
    colormapCombo->addItem(QIcon(pixmap),QString(),colormap);
  }
  colormapCombo->setCurrentIndex(colormapCombo->findData(imageView->colormap()));
  toolbar->addWidget(colormapCombo);
  /* Add radio buttons for the scale type */
  QWidget * scaleBox = new QWidget(toolbar);
  vbox = new QVBoxLayout(scaleBox);
  scaleBox->setLayout(vbox);
  QRadioButton * rb = new QRadioButton("Linear",scaleBox);
  rb->setToolTip(tr("Set Linear Scale"));
  QPalette p = QPalette(rb->palette());
  p.setColor(QPalette::WindowText,Qt::white);
  rb->setPalette(p);
  rb->setChecked(true);
  vbox->addWidget(rb);
  rb = new QRadioButton("Log",scaleBox);
  p = QPalette(rb->palette());
  p.setColor(QPalette::WindowText,Qt::white);
  rb->setPalette(p);
  rb->setToolTip(tr("Set Logarythmic Scale"));
  vbox->addWidget(rb);
  connect(rb,SIGNAL(toggled(bool)),imageView,SLOT(logScale(bool)));
  toolbar->addWidget(scaleBox);
  QAction * maxContrastImage = new QAction(QIcon(":images/bricontrast.png"),tr("&Maximize Contrast"), toolbar);
  maxContrastImage->setStatusTip(tr("Maximizes the contrast of the selected image."));
  connect(maxContrastImage,SIGNAL(triggered(bool)),imageView,SLOT(maxContrast()));
  toolbar->addAction(maxContrastImage);
  QAction * loadImage = new QAction(QIcon(":images/fileopen.png"),tr("&Load Image"), toolbar);
  loadImage->setStatusTip(tr("Load image file."));
  connect(loadImage,SIGNAL(triggered(bool)),imageView,SLOT(loadUserSelectedImage()));
  toolbar->addAction(loadImage);
  QAction * shiftImage = new QAction(QIcon(":images/crossing_arrows.png"),tr("&Shift Image"), this);
  shiftImage->setStatusTip(tr("Shifts the quadrants of the selected image."));
  connect(shiftImage,SIGNAL(triggered(bool)),imageView,SLOT(shiftImage()));
  toolbar->addAction(shiftImage);

  stretcher = new QWidget(toolbar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  toolbar->addWidget(stretcher);  
  
  setMinimumSize(0,sizeHint().height());
  //    frame->hide();
  visibilityTimer.setSingleShot(true);
  visibilityTimer.setInterval(200);
  connect(&visibilityTimer,SIGNAL(timeout()),this,SLOT(changeVisibility()));
  
  underMouse.append(this);
  QList<QWidget *> children = findChildren<QWidget *>();
  for(int i = 0;i<children.size();i++){
    underMouse.append(children[i]);
  }
}


bool ImageViewPanel::eventFilter(QObject * w,QEvent * e){
    if(e->type() == QEvent::Enter){
      //        underMouse.append(w);
	visibilityTimer.start();
    }
    if(e->type() == QEvent::Leave){
	visibilityTimer.start();
    }
    if(e->type() == QEvent::ChildAdded){
      QChildEvent * ce = (QChildEvent *)e;
      ce->child()->installEventFilter(this);
    }
    return false;
}

void ImageViewPanel::changeVisibility(){
  QPoint mousePos = QCursor::pos();
  QWidget * widgetUnder = QApplication::widgetAt(mousePos);
  if(underMouse.contains(widgetUnder)){
    frame->show();
  }else{
    frame->hide();
  }
}


void ImageViewPanel::changeColormap(int index){
  if(index < 0){
    return;
  }
  int colormap = colormapCombo->itemData(index).toInt();
  imageView->setColormap(colormap);
}

void ImageViewPanel::onDisplayComboChanged(int index){
  if(index < 0){
    return;
  }
  int display = displayCombo->itemData(index).toInt();
  imageView->setDisplay(display);
}
