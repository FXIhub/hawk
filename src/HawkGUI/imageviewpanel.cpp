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
  vbox->setContentsMargins(0,0,0,0);
  setLayout(vbox);
  //   frame = new QFrame(this);
  //  stretcher->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding));
  //  vbox->addWidget(stretcher);
  frame = new QScrollArea(this);
  frame->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  frame->setObjectName("panelFrame");
  frame->setFrameStyle(QFrame::NoFrame);
  vbox->addWidget(frame);
  //  frame->setLayout(vbox);
  
  QFrame * toolbar = new QFrame(frame);

  vbox = new QVBoxLayout(toolbar);
  vbox->setContentsMargins(0,0,0,0);
  QHBoxLayout * htop = new QHBoxLayout();
  QHBoxLayout * hbottom = new QHBoxLayout();
  vbox->addLayout(htop);
  vbox->addLayout(hbottom);
  htop->addStretch();
  hbottom->addStretch();

  toolbar->setLayout(vbox);
  vbox->setContentsMargins(0,0,0,0);
  htop->setContentsMargins(0,0,0,0);
  hbottom->setContentsMargins(0,0,0,0);

  toolbar->setObjectName("panelToolBar");
  QSize iconSize = QSize(22,22);


  saveImage = new QToolButton(toolbar);
  saveImage->setIcon(QIcon(":images/filesave.png"));
  saveImage->setToolTip(tr("Save Image"));
  htop->addWidget(saveImage);
  saveImage->hide();
  connect(saveImage,SIGNAL(clicked()),imageView,SLOT(saveImage()));
  saveImage->setIconSize(iconSize);
  QToolButton *  loadImage= new QToolButton(toolbar);
  loadImage->setIcon(QIcon(":images/fileopen.png"));
  loadImage->setToolTip(tr("Load Image"));
  htop->addWidget(loadImage);
  connect(loadImage,SIGNAL(clicked()),imageView,SLOT(loadUserSelectedImage()));
  loadImage->setIconSize(iconSize);

  displayCombo = new QComboBox; 
  displayCombo->setMinimumContentsLength(8);
  displayCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  displayCombo->setToolTip(tr("Select display class"));
  displayCombo->addItem("Amplitudes",0);
  displayCombo->addItem("Phases",SpColormapPhase);
  displayCombo->addItem("Mask",SpColormapMask);
  displayCombo->addItem("Shaded Mask",SpColormapShadedMask);
  displayCombo->view()->setAutoScrollMargin(0);
  displayCombo->view()->setAutoScroll(false);
  connect(displayCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(onDisplayComboChanged(int)));
  htop->addWidget(displayCombo);

  colormapCombo = new QComboBox(toolbar);
  colormapCombo->setToolTip(tr("Select Colormap"));
  connect(colormapCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(changeColormap(int)));
  colormapCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
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
  htop->addWidget(colormapCombo);

  logPush = new QToolButton(toolbar);
  logPush->setIcon(QIcon(":images/log_scale.png"));
  logPush->setCheckable(true);
  logPush->setChecked(false);
  logPush->setToolTip(tr("Toggle Logarythmic Scale"));
  logPush->setIconSize(iconSize);
  connect(logPush,SIGNAL(toggled(bool)),imageView,SLOT(setLogScale(bool)));
  hbottom->addWidget(logPush);

  
  QToolButton * maxContrastImage = new QToolButton(toolbar);
  maxContrastImage->setIcon(QIcon(":images/bricontrast.png"));
  maxContrastImage->setToolTip(tr("Maximize Contrast"));
  hbottom->addWidget(maxContrastImage);
  connect(maxContrastImage,SIGNAL(clicked()),imageView,SLOT(maxContrast()));
  maxContrastImage->setIconSize(iconSize);
  QToolButton * shiftImage = new QToolButton(toolbar);
  shiftImage->setIcon(QIcon(":images/crossing_arrows.png"));
  shiftImage->setToolTip(tr("Shift Image"));
  hbottom->addWidget(shiftImage);
  connect(shiftImage,SIGNAL(clicked()),imageView,SLOT(shiftImage()));
  shiftImage->setIconSize(iconSize);
  QToolButton * fourierTransformImage = new QToolButton(toolbar);
  fourierTransformImage->setIcon(QIcon(":images/fourier_transform.png"));
  fourierTransformImage->setToolTip(tr("Fourier Transforms the part of the image currently visible."));
  hbottom->addWidget(fourierTransformImage);
  connect(fourierTransformImage,SIGNAL(clicked()),imageView,SLOT(fourierTransform()));
  fourierTransformImage->setIconSize(iconSize);
  QToolButton * fourierTransformSquaredImage = new QToolButton(toolbar);
  fourierTransformSquaredImage->setIcon(QIcon(":images/fourier_transform_squared.png"));
  fourierTransformSquaredImage->setToolTip(tr("Fourier Transforms the square of the part of the image currently visible."));
  hbottom->addWidget(fourierTransformSquaredImage);
  connect(fourierTransformSquaredImage,SIGNAL(clicked()),imageView,SLOT(fourierTransformSquared()));
  fourierTransformSquaredImage->setIconSize(iconSize);

  QToolButton * stickyButton = new QToolButton(toolbar);
  stickyButton->setIcon(QIcon(":images/push_pin.png"));
  stickyButton->setToolTip(tr("Keeps the panel always visible."));
  QPalette p(stickyButton->palette());
  p.setColor(QPalette::Window,Qt::transparent);
  p.setColor(QPalette::Button,Qt::transparent);
  p.setColor(QPalette::Foreground,Qt::transparent);
  p.setColor(QPalette::Text,Qt::transparent);
  stickyButton->setPalette(p);

  hbottom->addWidget(stickyButton);
  stickyButton->setCheckable(true);
  stickyButton->setChecked(false);
  connect(stickyButton,SIGNAL(toggled(bool)),this,SLOT(setSticky(bool)));
  stickyButton->setIconSize(iconSize);

  hbottom->addStretch();
  htop->addStretch();

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

  connect(imageView,SIGNAL(imageLoaded(QString)),this,SLOT(onImageLoaded()));
  setSticky(false);
  setCursor(QCursor(Qt::ArrowCursor));
}


bool ImageViewPanel::eventFilter(QObject * ,QEvent * e){
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
  if(!sticky()){
    if(underMouse.contains(widgetUnder)){
      frame->show();
    }else{
      frame->hide();
    }
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

void ImageViewPanel::onImageLoaded(){
  colormapCombo->setCurrentIndex(colormapCombo->findData(imageView->colormap()));
  displayCombo->setCurrentIndex(displayCombo->findData(imageView->display()));
  logPush->setChecked(imageView->logScale());
}


void ImageViewPanel::setVisibility(bool visible){
  if(visible){
    frame->show();
  }else{
    frame->hide();
  }
}

void ImageViewPanel::setSticky(bool sticky){
  mySticky = sticky;
}

bool ImageViewPanel::sticky() const{
 return mySticky;
}

void ImageViewPanel::showSaveButton(bool show){
  if(show){
    saveImage->show();
  }else{
    saveImage->hide();
  }
}
