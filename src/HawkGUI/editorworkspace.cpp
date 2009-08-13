#include <QtGui>

#include "editorworkspace.h"
#include "imageeditorview.h"
#include "imageviewpanel.h"
#include "imageitem.h"
#include "math_parser.h"

EditorWorkspace::EditorWorkspace(QWidget * parent)
  :QWidget(parent)
{
  QHBoxLayout * hbox = new QHBoxLayout(this);
  this->setLayout(hbox);
  QSplitter * leftSplitter = new QSplitter(Qt::Vertical,this);
  hbox->addWidget(leftSplitter);
  editorView = new ImageEditorView(this);  
  hbox->addWidget(editorView);
  leftSplitter->addWidget(createTools());
  leftSplitter->addWidget(createPropertiesTree());
  connect(editorView,SIGNAL(imageItemChanged(ImageItem *)),this,SLOT(loadProperties()));
  editorView->imageViewPanel()->setVisibility(true);
  editorView->imageViewPanel()->showSaveButton(true);  
}


QWidget * EditorWorkspace::createTools(){
  QGroupBox * toolBox = new QGroupBox(tr("Tools"),this);
  QGridLayout * layout = new QGridLayout(toolBox);
  toolBox->setLayout(layout);
  QSize iconSize = QSize(22,22);
  QToolButton * pointer = new QToolButton(toolBox);
  pointer->setIcon(QIcon(":images/cursor_arrow.png"));
  pointer->setToolTip(tr("Drag/Scale Image\n(Shift to activate)"));
  pointer->setIconSize(iconSize);
  pointer->setCheckable(true);
  pointer->setAutoExclusive(true);
  if(editorView->editorMode() == EditorDefaultMode){
    pointer->setChecked(true);
  }
  connect(pointer,SIGNAL(clicked(bool)),this,SLOT(onPointerClicked()));
  layout->addWidget(pointer,0,0);
  QToolButton * bullseye = new QToolButton(toolBox);
  bullseye->setIcon(QIcon(":images/bullseye.png"));
  bullseye->setToolTip(tr("Set Image Center"));
  bullseye->setIconSize(iconSize);
  bullseye->setCheckable(true);
  bullseye->setAutoExclusive(true);
  connect(bullseye,SIGNAL(clicked(bool)),this,SLOT(onBullseyeClicked()));
  layout->addWidget(bullseye,0,1);
  QToolButton * drop = new QToolButton(toolBox);
  drop->setIcon(QIcon(":images/water_drop.png"));
  drop->setToolTip(tr("Blur Image"));
  drop->setIconSize(iconSize);
  drop->setCheckable(true);
  drop->setAutoExclusive(true);
  if(editorView->editorMode() == EditorBlurMode){
    drop->setChecked(true);
  }
  connect(drop,SIGNAL(clicked(bool)),this,SLOT(onDropClicked()));
  layout->addWidget(drop,0,2);
  QToolButton * mathEdit = new QToolButton(toolBox);
  mathEdit->setIcon(QIcon(":images/formula_pi.png"));
  mathEdit->setToolTip(tr("Evaluate Expression"));
  mathEdit->setIconSize(iconSize);
  connect(mathEdit,SIGNAL(clicked(bool)),this,SLOT(onMathEdit()));
  layout->addWidget(mathEdit,0,3);
  QToolButton * filter = new QToolButton(toolBox);
  filter->setIcon(QIcon(":images/optical_filter.png"));
  filter->setToolTip(tr("Filter Image"));
  filter->setIconSize(iconSize);
  connect(filter,SIGNAL(clicked(bool)),this,SLOT(onFilterClicked()));
  layout->addWidget(filter,0,4);
  QToolButton * selection = new QToolButton(toolBox);
  selection->setIcon(QIcon(":images/selection.png"));
  selection->setToolTip(tr("Select image section"));
  selection->setIconSize(iconSize);
  selection->setCheckable(true);
  selection->setAutoExclusive(true);
  connect(selection,SIGNAL(clicked(bool)),this,SLOT(onSelectionClicked()));
  layout->addWidget(selection,0,5);

  QToolButton * lineout = new QToolButton(toolBox);
  lineout->setIcon(QIcon(":images/lineout_plot.png"));
  lineout->setToolTip(tr("Trace plot lineout"));
  lineout->setIconSize(iconSize);
  lineout->setCheckable(true);
  lineout->setAutoExclusive(true);
  connect(lineout,SIGNAL(clicked(bool)),this,SLOT(onLineoutClicked()));
  layout->addWidget(lineout,0,6);

  //  connect(mathEdit,SIGNAL(clicked(bool)),this,SLOT(onMathEdit()));
  
  toolOptions = new QWidget(toolBox);
  QVBoxLayout * vbox = new QVBoxLayout(toolOptions);
  toolOptions->setLayout(vbox);
  QFrame * separator = new QFrame(toolOptions);
  separator->setFrameStyle(QFrame::HLine|QFrame::Raised);
  separator->setLineWidth(1);


  vbox->addWidget(separator);
  toolOptionsLayout = new QStackedLayout();
  vbox->addLayout(toolOptionsLayout);
  toolOptionsLayout->addWidget(new QWidget(toolOptions));
  toolOptionsLayout->addWidget(new QWidget(toolOptions));

  dropToolOptions = new QWidget(toolOptions);
  QGridLayout * grid = new QGridLayout(dropToolOptions);
  dropToolOptions->setLayout(grid);
  grid->addWidget(new QLabel(tr("Brush Radius:"),dropToolOptions),0,0);
  QDoubleSpinBox * spinBox = new QDoubleSpinBox(dropToolOptions);
  connect(spinBox,SIGNAL(valueChanged(double)),editorView,SLOT(setDropBrushRadius(double)));
  spinBox->setMinimum(0);
  spinBox->setValue(editorView->getDropBrushRadius());
  grid->addWidget(spinBox,0,1);
  grid->addWidget(new QLabel(tr("Blur Radius:"),dropToolOptions),1,0);
  spinBox = new QDoubleSpinBox(dropToolOptions);
  spinBox->setMinimum(0);
  spinBox->setValue(editorView->getDropBlurRadius());
  connect(spinBox,SIGNAL(valueChanged(double)),editorView,SLOT(setDropBlurRadius(double)));
  grid->addWidget(spinBox,1,1);
  toolOptionsLayout->addWidget(dropToolOptions);

  filterToolOptions = new QWidget(toolOptions);
  grid = new QGridLayout(filterToolOptions);
  filterToolOptions->setLayout(grid);
  grid->addWidget(new QLabel(tr("Filter Type:"),filterToolOptions),0,0);
  QComboBox * comboBox = new QComboBox(filterToolOptions);
  comboBox->addItem("Gaussian Radial");
  comboBox->addItem("Horizontal bands removal");
  comboBox->setMinimumContentsLength(10);
  comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  grid->addWidget(comboBox,0,1);
  grid->setRowStretch(2,100);
  grid->setColumnStretch(2,100);
  toolOptionsLayout->addWidget(filterToolOptions);

  selectionToolOptions = new QWidget(toolOptions);
  grid = new QGridLayout(selectionToolOptions);
  selectionToolOptions->setLayout(grid);
  grid->addWidget(new QLabel("Mode:",selectionToolOptions),0,0);
  QToolButton *  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection.png"));
  button->setToolTip("Set selection");
  button->setCheckable(true);
  button->setChecked(true);
  button->setAutoExclusive(true);
  grid->addWidget(button,0,1);
  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection_union.png"));
  button->setToolTip("Add to selection");
  button->setCheckable(true);
  button->setAutoExclusive(true);
  grid->addWidget(button,0,2);
  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection_subtract.png"));
  button->setToolTip("Remove from selection");  
  button->setCheckable(true);
  button->setAutoExclusive(true);
  grid->addWidget(button,0,3);
  grid->setRowStretch(5,100);
  grid->setColumnStretch(5,100);
  toolOptionsLayout->addWidget(selectionToolOptions);


  toolOptions->hide();
  layout->addWidget(toolOptions,1,0,1,11);
  layout->setColumnStretch(11,100);
  layout->setRowStretch(3,100);
  return toolBox;
}

void EditorWorkspace::loadProperties(){
  const QString tag("HawkImage_");
  const QMetaObject *metaobject = editorView->metaObject();
  QStandardItemModel * model = qobject_cast<QStandardItemModel *>(propertiesTree->model());
  model->clear();
  model->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
  int count = metaobject->propertyCount();
  for (int i=0; i<count; ++i) {
     QMetaProperty metaproperty = metaobject->property(i);
     const char *name = metaproperty.name();
     /* 
	This is a prefix to distinguish Hawk Image properties from the
	normal widget properties 
     */
     if(!QString(name).startsWith(tag)){
       continue;
     }
     QVariant var =  editorView->property(name);
     QStandardItem *parentItem = model->invisibleRootItem();
     Qt::ItemFlags itemValueFlags = Qt::ItemIsSelectable|Qt::ItemIsEnabled;
     if(metaproperty.isWritable()){
       itemValueFlags |= Qt::ItemIsEditable;
     }
     if(var.type() == QVariant::PointF){
       QPointF value = var.toPointF();
       QStandardItem * itemName = new QStandardItem(editorView->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0 x %1").arg(value.x()).arg(value.y()));
       itemName->setData(value,Qt::UserRole + 1);
       itemName->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags & ~Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       parentItem = itemName;
       itemName = new QStandardItem("x");
       itemValue = new QStandardItem(QString("%0").arg(value.x()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       itemName = new QStandardItem("y");
       itemValue = new QStandardItem(QString("%0").arg(value.y()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Size){
       QSize value = var.toSize();
       QStandardItem * itemName = new QStandardItem(editorView->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0 x %1").arg(value.width()).arg(value.height()));
       itemName->setData(value,Qt::UserRole + 1);
       itemName->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags & ~Qt::ItemIsEditable);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       parentItem = itemName;
       itemName = new QStandardItem("width");
       itemValue = new QStandardItem(QString("%0").arg(value.width()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
       itemName = new QStandardItem("height");
       itemValue = new QStandardItem(QString("%0").arg(value.height()));
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Bool){
       bool value = var.toBool();
       QStandardItem * itemName = new QStandardItem(editorView->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem();
       itemValue->setData(value,Qt::UserRole + 1);
       itemValue->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       if(itemValueFlags & Qt::ItemIsEditable){
	 itemValue->setFlags((itemValueFlags | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
       }else{
	 itemValue->setFlags(itemValueFlags);
       }
       if(value){
	 itemValue->setCheckState(Qt::Checked);
       }else{
	 itemValue->setCheckState(Qt::Unchecked);
       }
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }else if(var.type() == QVariant::Double){
       double value = var.toDouble();
       QStandardItem * itemName = new QStandardItem(editorView->propertyNameToDisplayName(name));
       QStandardItem * itemValue = new QStandardItem(QString("%0").arg(value));
       itemValue->setData(value,Qt::UserRole + 1);
       itemValue->setData(QString(name),Qt::UserRole + 2);
       itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);
       itemValue->setFlags(itemValueFlags);
       parentItem->appendRow(QList<QStandardItem *>() << itemName << itemValue);
     }
  }
}

QTreeView * EditorWorkspace::createPropertiesTree(){
  /* need a real QTreeView with a model */
  QStandardItemModel * model = new QStandardItemModel;
  QTreeView *treeView = new QTreeView(this);
  propertiesTree = treeView;
  treeView->setModel(model);
  loadProperties();
  treeView->setAlternatingRowColors(true);
  treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  treeView->resizeColumnToContents(0);
  treeView->resizeColumnToContents(1);
  connect(model,SIGNAL(itemChanged(QStandardItem * )),this,SLOT(onItemChanged(QStandardItem *)));
  return treeView;  
}


void EditorWorkspace::clicked(QModelIndex index){
  if(index.column() == 1){
    qDebug("clicked to edit");
  }
}


void EditorWorkspace::onItemChanged(QStandardItem * item){
  if(((item->flags() & Qt::ItemIsEditable) || (item->flags() & Qt::ItemIsUserCheckable )) == 0){
    return;
  }
  qDebug("item changed");
  if(!item->data().isValid()){
    item = item->parent();
  }
  if(!item->data().isValid()){
    qFatal("Can't reach here");
  }
  QVariant var = item->data();
  if(var.type() == QVariant::PointF){
    /* collect new value from the children */
    if(item->hasChildren()){
      QStandardItem * xItem = item->child(0,1);
      QStandardItem * yItem = item->child(1,1);
      QStandardItem * combined = item->model()->item(item->row(),item->column()+1);
      QPointF value = QPointF(xItem->text().toDouble(),yItem->text().toDouble());
      QString property = item->data(Qt::UserRole + 2).toString();
      editorView->setProperty(property.toAscii().constData(),value);
      combined->setText(QString("%0 x %1").arg(value.x()).arg(value.y()));
      
      qDebug("New value for %s = %f %f",property.toAscii().data(),value.x(),value.y());
    }else{
      qFatal("Can't reach here");
    }
  }else if(var.type() == QVariant::Size){
    /* collect new value from the children */
    if(item->hasChildren()){
      QStandardItem * xItem = item->child(0,1);
      QStandardItem * yItem = item->child(1,1);
      QStandardItem * combined = item->model()->item(item->row(),item->column()+1);
      QSize value = QSize(xItem->text().toInt(),yItem->text().toInt());
      QString property = item->data(Qt::UserRole + 2).toString();
      editorView->setProperty(property.toAscii().constData(),value);
      combined->setText(QString("%0 x %1").arg(value.width()).arg(value.width()));
      
      qDebug("New value for %s = %d x %d",property.toAscii().data(),value.width(),value.height());
    }else{
      qFatal("Can't reach here");
    }
  }else if(var.type() == QVariant::Double){
    double value = item->text().toDouble();
    QString property = item->data(Qt::UserRole + 2).toString();
    editorView->setProperty(property.toAscii().constData(),value);
    item->setText(QString("%0").arg(value));		  
    qDebug("New value for %s = %f",property.toAscii().data(),value);
  }else if(var.type() == QVariant::Bool){
    bool value = (item->checkState() == Qt::Checked);
    QString property = item->data(Qt::UserRole + 2).toString();
    editorView->setProperty(property.toAscii().constData(),value);
    qDebug("New value for %s = %d",property.toAscii().data(),value);
  }
}


void EditorWorkspace::onMathEdit(){
  qDebug("here");
  bool ok;
  QString text = QInputDialog::getText(this, tr("Apply expression to image"),
				       tr("\"A\" represents the current image. Examples:\n"
					  "A + 3 -> adds 3 to every pixel\n"
					  "fft(A) -> takes the fourier transform of the image\n"
					  "\n"
					  "Expression:"), 
				       QLineEdit::Normal,
				       "A + 0", &ok);
  if (ok && !text.isEmpty()){
    Image * image_list[2] = {0,0};
    if(editorView->imageItem() && editorView->imageItem()->getImage()){
      image_list[0] = editorView->imageItem()->getImage();
    }
    qDebug("Got formula %s\n",text.toAscii().data());
    Math_Output * out = evaluate_math_expression(text.toAscii().data(),image_list);
    if(out->type == MathOutputImage){
      /* We got a new image */
      ImageItem * item = new ImageItem(out->image,QString());
      editorView->setImage(item);
    }else if(out->type == MathOutputScalar){
      /* We got a scalar. Show it to the user */
      QString out_text;
      out_text = QString("Expression evaluated to %0 + %1 i").arg(sp_real(out->scalar)).arg(sp_imag(out->scalar));
      QMessageBox::information ( 0,tr("Apply expression to image"),out_text);
    }else if(out->type == MathOutputError){
      QString out_text(out->error_msg);
      QMessageBox::warning( 0,tr("Apply expression to image"),out_text);

    }
  }
}



void EditorWorkspace::onPointerClicked(){
  editorView->setDefaultMode();
  toolOptions->hide();
}

void EditorWorkspace::onBullseyeClicked(){
  toolOptions->hide();
}

void EditorWorkspace::onDropClicked(){
  editorView->setBlurMode();
  toolOptionsLayout->setCurrentWidget(dropToolOptions);
  toolOptions->show();
}

void EditorWorkspace::onFilterClicked(){
  toolOptionsLayout->setCurrentWidget(filterToolOptions);
  toolOptions->show();
}

void EditorWorkspace::onSelectionClicked(){
  editorView->setSelectionMode();
  toolOptionsLayout->setCurrentWidget(selectionToolOptions);
  toolOptions->show();
}

void EditorWorkspace::onLineoutClicked(){
  editorView->setLineoutMode();
  toolOptions->hide();
}


