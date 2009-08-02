#include "imageviewpanel.h"
#include "imageview.h"
#include "spimage.h"
#include <QtGui>

ImageViewPanel::ImageViewPanel(ImageView * parent)
  :QWidget(parent)
{
    frame = NULL;
  installEventFilter(this);
  QVBoxLayout * vbox = new QVBoxLayout(this);
  setLayout(vbox);
  vbox->setContentsMargins(0,0,0,0);
   frame = new QFrame(this);
  frame->setObjectName("panelFrame");
//   frame->setStyleSheet("QFrame#panelFrame{"
//                   "border: 2px solid #26466D;"
//                   "border-radius: 6px;"
//                   "padding: 2px;"
// "background: rgba(0,0,0,100);"
//                   "background-color: rgba(0,0,0,100);"
//                   "}"
//                   );
    vbox->addWidget(frame);
    //    frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    vbox = new QVBoxLayout(frame);
    frame->setLayout(vbox);
    vbox->setContentsMargins(0,0,0,0);
    QToolBar * toolbar = new QToolBar(frame);
  toolbar->setStyleSheet("QToolBar{"
			 "border: 2px solid #26466D;"
			 "border-radius: 6px;"
			 "padding: 4px;"
			 "background-color: rgba(0,0,0,100);"
			 "}"
			 );
    vbox->addWidget(toolbar);
    QComboBox * cb = new QComboBox(toolbar);
    /*    cb->setStyleSheet("QComboBox{"
		      "background: rgba(0,0,0,100);"
		      ""
		      "}"
		      "QComboBox QAbstractItemView {"
		      "border: 2px solid darkgray;"
		      "selection-background-color: rgba(0,0,0,100);"
		      "background-color: rgb(0,0,0,100);"
		      "}"
		      
		      "QComboBox::drop-down {"
		      "background-color: rgb(0,0,0,100);"
		      " }");*/
    //    QPushButton * push = new QPushButton("Push me!",toolbar);
    //    QCheckBox * check = new QCheckBox("Check",toolbar);
    /* create combobox for the colormap */
    cb->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    QSize iconSize = QSize(16,16);
    for(int colormap =  COLOR_GRAYSCALE;colormap <= COLOR_WHEEL;colormap*=2){
      QLinearGradient gradient = QLinearGradient(QPointF(0,0),QPointF(iconSize.width(),0));
      for(int step = 0;step < 10;step++){
	real value = step/10.0;
	sp_rgb rgb = sp_colormap_rgb_from_value(value,colormap);
	gradient.setColorAt(value,QColor(rgb.r,rgb.g,rgb.b));			
      }
      QPixmap pixmap = QPixmap(iconSize);
      QPainter painter(&pixmap);
      painter.fillRect(0, 0, iconSize.width(), iconSize.height(), gradient);
      
      cb->addItem(QIcon(pixmap),QString());
    }
    //    cb->addItem("first");
    //    cb->addItem("second");
    //    QHBoxLayout * hbox = new QHBoxLayout(toolbar);
    //    toolbar->setLayout(hbox);
    //    hbox->addStretch();
    toolbar->addWidget(cb);
    //    toolbar->addWidget(push);
    //    toolbar->addWidget(check);

    //    QAction * logScaleImage = new QAction(QIcon(":images/log_scale.png"),tr("&Log Scale"), this);
    //    logScaleImage->setStatusTip(tr("Toggles log scale on the selected image."));
    //    logScaleImage->setCheckable(true);
    //    connect(logScaleImage,SIGNAL(toggled(bool)),parent,SLOT(logScale(bool)));
    //    toolbar->addAction(logScaleImage);
    /* Add radio buttons for the scale type */
    QGroupBox * scaleBox = new QGroupBox(toolbar);
    vbox = new QVBoxLayout(scaleBox);
    scaleBox->setLayout(vbox);
    QRadioButton * rb = new QRadioButton("Linear",scaleBox);
    rb->setChecked(true);
    vbox->addWidget(rb);
    rb = new QRadioButton("Log",scaleBox);
    vbox->addWidget(rb);
    connect(rb,SIGNAL(toggled(bool)),parent,SLOT(logScale(bool)));
    toolbar->addWidget(scaleBox);
    QAction * maxContrastImage = new QAction(QIcon(":images/bricontrast.png"),tr("&Maximize Contrast"), toolbar);
    maxContrastImage->setStatusTip(tr("Maximizes the contrast of the selected image."));
    connect(maxContrastImage,SIGNAL(triggered(bool)),parent,SLOT(maxContrast()));
    toolbar->addAction(maxContrastImage);

    //    hbox->addStretch();
    setMinimumSize(0,sizeHint().height());
    frame->hide();
}


bool ImageViewPanel::eventFilter(QObject * w,QEvent * e){
    if(e->type() == QEvent::Enter){
        underMouse.append(w);
        QTimer::singleShot(200, this, SLOT(changeVisibility()));
    }
    if(e->type() == QEvent::Leave){
        underMouse.removeAll(w);
        QTimer::singleShot(200, this, SLOT(changeVisibility()));
    }
    if(e->type() == QEvent::ChildAdded){
        QChildEvent * ce = (QChildEvent *)e;
        ce->child()->installEventFilter(this);
    }
    return false;
}

void ImageViewPanel::changeVisibility(){
    if(underMouse.size()){
        frame->show();
    }else{
        frame->hide();
    }
}
