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
  QWidget * stretcher = new QWidget(this);
  setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
  //  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding));
  //  vbox->addWidget(stretcher);
  frame = new QScrollArea(this);
  frame->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  frame->setObjectName("panelFrame");
  frame->setFrameStyle(QFrame::NoFrame);
  vbox->addWidget(frame);
  QGridLayout * grid = new QGridLayout(frame);
  //  frame->setLayout(vbox);
  
  QFrame * toolbar = new QFrame(frame);
  toolbar->setLayout(grid);
  grid->setContentsMargins(0,0,0,0);

  toolbar->setObjectName("panelToolBar");

  //  vbox->addWidget(toolbar);
   stretcher = new QWidget(toolbar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  grid->addWidget(stretcher,0,0);
  displayCombo = new QComboBox; 
  displayCombo->setMinimumContentsLength(4);
  displayCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  displayCombo->addItem("Amplitudes",0);
  displayCombo->addItem("Phases",SpColormapPhase);
  displayCombo->addItem("Mask",SpColormapMask);
  connect(displayCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onDisplayComboChanged(int)));
  grid->addWidget(displayCombo,0,1);
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
  grid->addWidget(colormapCombo,0,2);

  /* Add radio buttons for the scale type */
  /*  QWidget * scaleBox = new QWidget(toolbar);
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
  hbox->addWidget(scaleBox);
  */
  QCheckBox * logCheck = new QCheckBox(tr("log"),toolbar);
  logCheck->setToolTip(tr("Toggle Logarythmic Scale"));
  connect(logCheck,SIGNAL(toggled(bool)),imageView,SLOT(logScale(bool)));
  grid->addWidget(logCheck,0,3);

  
  QToolButton * maxContrastImage = new QToolButton(toolbar);
  maxContrastImage->setIcon(QIcon(":images/bricontrast.png"));
  maxContrastImage->setToolTip(tr("Maximize Contrast"));
  grid->addWidget(maxContrastImage,0,4);
  connect(maxContrastImage,SIGNAL(toggled(bool)),imageView,SLOT(maxContrast()));
  QToolButton *  loadImage= new QToolButton(toolbar);
  loadImage->setIcon(QIcon(":images/fileopen.png"));
  loadImage->setToolTip(tr("Load Image"));
  grid->addWidget(loadImage,0,5);
  connect(loadImage,SIGNAL(toggled(bool)),imageView,SLOT(loadUserSelectedImage()));
  QToolButton * shiftImage = new QToolButton(toolbar);
  shiftImage->setIcon(QIcon(":images/crossing_arrows.png"));
  shiftImage->setToolTip(tr("Shift Image"));
  grid->addWidget(shiftImage,0,6);
  connect(shiftImage,SIGNAL(toggled(bool)),imageView,SLOT(shiftImage()));
  stretcher = new QWidget(toolbar);
  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  grid->addWidget(stretcher,0,7);  
  setFixedHeight(toolbar->sizeHint().height()+frame->horizontalScrollBar()->sizeHint().height());
  toolbar->setMinimumWidth(toolbar->sizeHint().width());
  frame->hide();

  frame->setWidget(toolbar);
  frame->setWidgetResizable(true);
  frame->setAlignment(Qt::AlignBottom);

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
