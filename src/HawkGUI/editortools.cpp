#include "editortools.h"
#include "editorworkspace.h"
#include "imageeditorview.h"
#include "math_parser.h"
#include "imageitem.h"

#include <QtGui>


EditorTools::EditorTools(EditorWorkspace * parent)
  :QGroupBox((QWidget *)parent)
{
  editor = parent;
  setTitle(tr("Tools"));
  QGridLayout * layout = new QGridLayout(this);
  this->setLayout(layout);
  QSize iconSize = QSize(22,22);
  QToolButton * pointer = new QToolButton(this);
  pointer->setIcon(QIcon(":images/cursor_arrow.png"));
  pointer->setToolTip(tr("Drag/Scale Image\n(Shift to activate)"));
  pointer->setIconSize(iconSize);
  pointer->setCheckable(true);
  pointer->setAutoExclusive(true);
  if(editor->editorView()->editorMode() == EditorDefaultMode){
    pointer->setChecked(true);
  }
  connect(pointer,SIGNAL(clicked(bool)),this,SLOT(onPointerClicked()));
  layout->addWidget(pointer,0,0);
  QToolButton * bullseye = new QToolButton(this);
  bullseye->setIcon(QIcon(":images/bullseye.png"));
  bullseye->setToolTip(tr("Set Image Center"));
  bullseye->setIconSize(iconSize);
  bullseye->setCheckable(true);
  bullseye->setAutoExclusive(true);
  connect(bullseye,SIGNAL(toggled(bool)),this,SLOT(onBullseyeToggled(bool)));
  layout->addWidget(bullseye,0,1);
  QToolButton * drop = new QToolButton(this);
  drop->setIcon(QIcon(":images/water_drop.png"));
  drop->setToolTip(tr("Blur Image"));
  drop->setIconSize(iconSize);
  drop->setCheckable(true);
  drop->setAutoExclusive(true);
  if(editor->editorView()->editorMode() == EditorBlurMode){
    drop->setChecked(true);
  }
  connect(drop,SIGNAL(clicked(bool)),this,SLOT(onDropClicked()));
  layout->addWidget(drop,0,2);
  QToolButton * mathEdit = new QToolButton(this);
  mathEdit->setIcon(QIcon(":images/formula_pi.png"));
  mathEdit->setToolTip(tr("Evaluate Expression"));
  mathEdit->setIconSize(iconSize);
  connect(mathEdit,SIGNAL(clicked(bool)),this,SLOT(onMathEdit()));
  layout->addWidget(mathEdit,0,3);
  QToolButton * filter = new QToolButton(this);
  filter->setIcon(QIcon(":images/optical_filter.png"));
  filter->setToolTip(tr("Filter Image"));
  filter->setIconSize(iconSize);
  connect(filter,SIGNAL(clicked(bool)),this,SLOT(onFilterClicked()));
  layout->addWidget(filter,0,4);
  QToolButton * selection = new QToolButton(this);
  selection->setIcon(QIcon(":images/selection.png"));
  selection->setToolTip(tr("Select image section"));
  selection->setIconSize(iconSize);
  selection->setCheckable(true);
  selection->setAutoExclusive(true);
  connect(selection,SIGNAL(clicked(bool)),this,SLOT(onSelectionClicked()));
  layout->addWidget(selection,0,5);

  QToolButton * lineout = new QToolButton(this);
  lineout->setIcon(QIcon(":images/lineout_plot.png"));
  lineout->setToolTip(tr("Plot linear profile"));
  lineout->setIconSize(iconSize);
  lineout->setCheckable(true);
  lineout->setAutoExclusive(true);
  connect(lineout,SIGNAL(clicked(bool)),this,SLOT(onLineoutClicked()));
  layout->addWidget(lineout,0,6);

  QToolButton * undo = new QToolButton(this);
  undo->setIcon(QIcon(":images/undo.png"));
  undo->setToolTip(tr("Undo last edit"));
  undo->setIconSize(iconSize);
  connect(undo,SIGNAL(clicked(bool)),this,SLOT(onUndoClicked()));
  layout->addWidget(undo,1,0);

  QToolButton * redo = new QToolButton(this);
  redo->setIcon(QIcon(":images/redo.png"));
  redo->setToolTip(tr("Redo last undone edit"));
  redo->setIconSize(iconSize);
  connect(redo,SIGNAL(clicked(bool)),this,SLOT(onRedoClicked()));
  layout->addWidget(redo,1,1);

  QToolButton * removeElectronics = new QToolButton(this);
  removeElectronics->setIcon(QIcon(":images/hardware.png"));
  removeElectronics->setToolTip(tr("Remove electronic noise"));
  removeElectronics->setIconSize(iconSize);
  removeElectronics->setCheckable(true);
  removeElectronics->setAutoExclusive(true);
  connect(removeElectronics,SIGNAL(clicked(bool)),this,SLOT(onRemoveElectronicsClicked()));
  layout->addWidget(removeElectronics,1,2);

  QToolButton * xcamMagic = new QToolButton(this);
  xcamMagic->setIcon(QIcon(":images/xcam.png"));
  xcamMagic->setToolTip(tr("Remove overscan and noise from xcam data"));
  xcamMagic->setIconSize(iconSize);
  connect(xcamMagic,SIGNAL(clicked(bool)),this,SLOT(onXcamMagicClicked()));
  layout->addWidget(xcamMagic,1,3);

  QToolButton * fillEmpty = new QToolButton(this);
  fillEmpty->setIcon(QIcon(":images/magic.png"));
  fillEmpty->setToolTip(tr("Fill data on the missing regions by interpolation"));
  fillEmpty->setIconSize(iconSize);
  connect(fillEmpty,SIGNAL(clicked(bool)),this,SLOT(onFillEmptyClicked()));
  layout->addWidget(fillEmpty,1,4);

  QToolButton * crop = new QToolButton(this);
  crop->setIcon(QIcon(":images/crop.png"));
  crop->setToolTip(tr("Crop image to selection"));
  crop->setIconSize(iconSize);
  connect(crop,SIGNAL(clicked(bool)),this,SLOT(onCropClicked()));
  layout->addWidget(crop,1,5);


  //  connect(mathEdit,SIGNAL(clicked(bool)),this,SLOT(onMathEdit()));
  
  toolOptions = new QWidget(this);
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
  connect(spinBox,SIGNAL(valueChanged(double)),editor->editorView(),SLOT(setDropBrushRadius(double)));
  spinBox->setMinimum(0);
  spinBox->setValue(editor->editorView()->getDropBrushRadius());
  grid->addWidget(spinBox,0,1);
  grid->addWidget(new QLabel(tr("Blur Radius:"),dropToolOptions),1,0);
  spinBox = new QDoubleSpinBox(dropToolOptions);
  spinBox->setMinimum(0);
  spinBox->setValue(editor->editorView()->getDropBlurRadius());
  connect(spinBox,SIGNAL(valueChanged(double)),editor->editorView(),SLOT(setDropBlurRadius(double)));
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
  QLabel * label = new QLabel("Mode:",selectionToolOptions);
  label->setAlignment(Qt::AlignRight);
  grid->addWidget(label,0,0);
  QToolButton *  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection.png"));
  button->setToolTip("Set selection");
  button->setCheckable(true);
  button->setChecked(true);
  button->setAutoExclusive(true);
  connect(button,SIGNAL(clicked()),this,SLOT(setSelectionModeSet()));
  grid->addWidget(button,0,1);
  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection_union.png"));
  button->setToolTip("Add to selection");
  button->setCheckable(true);
  button->setAutoExclusive(true);
  connect(button,SIGNAL(clicked()),this,SLOT(setSelectionModeUnite()));
  grid->addWidget(button,0,2);
  button = new QToolButton(selectionToolOptions);
  button->setIcon(QIcon(":images/selection_subtract.png"));
  button->setToolTip("Remove from selection");  
  button->setCheckable(true);
  button->setAutoExclusive(true);
  connect(button,SIGNAL(clicked()),this,SLOT(setSelectionModeSubtract()));
  grid->addWidget(button,0,3);
  label = new QLabel("Expression:",selectionToolOptions);
  label->setAlignment(Qt::AlignRight);
  grid->addWidget(label,1,0);
  selectExpression = new QLineEdit("A > 10",selectionToolOptions);
  grid->addWidget(selectExpression,1,1,1,3);
  QPushButton * evalExpression = new QPushButton("Select by Expression",selectionToolOptions);
  connect(evalExpression,SIGNAL(clicked()),this,SLOT(onSelectByExpression()));
  grid->addWidget(evalExpression,2,0,1,4);
  grid->setRowStretch(5,100);
  grid->setColumnStretch(5,100);
  toolOptionsLayout->addWidget(selectionToolOptions);

  electronicsToolOptions = new QWidget(toolOptions);
  grid = new QGridLayout(electronicsToolOptions);
  electronicsToolOptions->setLayout(grid);
  QPushButton * push = new QPushButton("Remove Vertical Lines",electronicsToolOptions);
  grid->addWidget(push,0,0);
  connect(push,SIGNAL(clicked()),this,SLOT(onRemoveVerticalLinesClicked()));
  push = new QPushButton("Remove Horizontal Lines",electronicsToolOptions);
  grid->addWidget(push,1,0);
  connect(push,SIGNAL(clicked()),this,SLOT(onRemoveHorizontalLinesClicked()));
  toolOptionsLayout->addWidget(electronicsToolOptions);

  fillEmptyToolOptions = new QWidget(toolOptions);
  grid = new QGridLayout(fillEmptyToolOptions);
  fillEmptyToolOptions->setLayout(grid);
  grid->addWidget(new QLabel("Blur Radius (px)",0,0));
  fillBlurRadius =  new QDoubleSpinBox(fillEmptyToolOptions);
  grid->addWidget(fillBlurRadius,0,1);

  grid->addWidget(new QLabel("Iterations"),1,0);
  fillIterations =  new QSpinBox(fillEmptyToolOptions);
  fillIterations->setMinimum(1);
  grid->addWidget(fillIterations,1,1);

  grid->addWidget(new QLabel("Blur Kernel"),2,0);
  fillBlurKernel = new QComboBox(fillEmptyToolOptions);
  fillBlurKernel->addItem("Gaussian");
  fillBlurKernel->addItem("Sinc");
  grid->addWidget(fillBlurKernel,2,1);

  push = new QPushButton("Interpolate",fillEmptyToolOptions);
  grid->addWidget(push,3,0,1,2);
  connect(push,SIGNAL(clicked()),this,SLOT(onInterpolateEmptyClicked()));
  toolOptionsLayout->addWidget(fillEmptyToolOptions);



  toolOptions->hide();
  layout->addWidget(toolOptions,2,0,1,11);
  layout->setColumnStretch(11,100);
  layout->setRowStretch(3,100);
}


void EditorTools::onPointerClicked(){
  editor->editorView()->setDefaultMode();
  toolOptions->hide();
}

void EditorTools::onBullseyeToggled(bool checked){
  editor->editorView()->setBullseyeMode(checked);
  toolOptions->hide();
}

void EditorTools::onDropClicked(){
  editor->editorView()->setBlurMode();
  toolOptionsLayout->setCurrentWidget(dropToolOptions);
  toolOptions->show();
}

void EditorTools::onFilterClicked(){
  toolOptionsLayout->setCurrentWidget(filterToolOptions);
  toolOptions->show();
}

void EditorTools::onSelectionClicked(){
  editor->editorView()->setSelectionMode();
  toolOptionsLayout->setCurrentWidget(selectionToolOptions);
  toolOptions->show();
}

void EditorTools::onLineoutClicked(){
  editor->editorView()->setLineoutMode();
  toolOptions->hide();
}

void EditorTools::onMathEdit(){
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
    const Image * image_list[2] = {0,0};
    if(editor->editorView()->selectedImage() && editor->editorView()->selectedImage()->getImage()){
      image_list[0] = editor->editorView()->selectedImage()->getImage();
    }
    qDebug("Got formula %s\n",text.toAscii().data());
    Math_Output * out = evaluate_math_expression(text.toAscii().data(),image_list);
    if(out->type == MathOutputImage){
      /* We got a new image */
      ImageItem * item = new ImageItem(out->image,QString(),editor->editorView());
      editor->editorView()->setImage(item);
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

void EditorTools::onUndoClicked(){
  if(editor->editorView()->selectedImage()){
    editor->editorView()->selectedImage()->undoEditSteps();
  }
}

void EditorTools::onRedoClicked(){
  if(editor->editorView()->selectedImage()){
    editor->editorView()->selectedImage()->redoEditSteps();
  }
}

void EditorTools::onRemoveVerticalLinesClicked(){
  if(editor->editorView()->selectedImage()){
    QRegion selected = editor->editorView()->selectedRegion();
    /* Do vertical averaging */
    /* simplify selection */
    QRect rect = selected.boundingRect();
    editor->editorView()->selectedImage()->removeVerticalLines(rect);
  }
}

void EditorTools::onRemoveHorizontalLinesClicked(){
  if(editor->editorView()->selectedImage()){
    QRegion selected = editor->editorView()->selectedRegion();
    /* Do vertical averaging */
    /* simplify selection */
    QRect rect = selected.boundingRect();
    editor->editorView()->selectedImage()->removeHorizontalLines(rect);
  }
}

void EditorTools::onRemoveElectronicsClicked(){
  editor->editorView()->setSelectionMode();
  toolOptionsLayout->setCurrentWidget(electronicsToolOptions);
  toolOptions->show();  
}


EditorTools::SelectionMode EditorTools::selectionMode(){
  return _selectionMode;
}

void EditorTools::setSelectionModeSet(){
  _selectionMode = SelectionSet;
}

void EditorTools::setSelectionModeUnite(){
  _selectionMode = SelectionUnite;
}

void EditorTools::setSelectionModeSubtract(){
  _selectionMode = SelectionSubtract;
}

void EditorTools::onSelectByExpression(){
  QString exp = selectExpression->text();
  if(editor->editorView()->selectedImage()){
    const Image * list[2] = {editor->editorView()->selectedImage()->getImage(),0};
    Math_Output * out = evaluate_math_expression(exp.toAscii().data(),list);
    if(out->type == MathOutputImage){
      uchar * bitData = (uchar *)malloc(sizeof(uchar)*ceil(sp_image_size(out->image)/8.0));
      for(int i = 0;i<sp_image_size(out->image);i++){
	if(i % 8 == 0){
	  bitData[i/8] = 0;
	}
	if(sp_cabs(sp_image_get_by_index(out->image,i)) != 0){
	  bitData[i/8] |= (1<<(i % 8));
	}
      }
      QRegion region = QRegion(QBitmap::fromData(QSize(sp_image_x(out->image),sp_image_y(out->image)),bitData,QImage::Format_MonoLSB));
      editor->editorView()->selectRegion(region);
    }    
  }  
}

void EditorTools::onXcamMagicClicked(){
  if(editor->editorView()->selectedImage()){
    editor->editorView()->selectedImage()->xcamPreprocess();
  }
}

void EditorTools::onFillEmptyClicked(){
  toolOptionsLayout->setCurrentWidget(fillEmptyToolOptions);
  toolOptions->show();
}

void EditorTools::onInterpolateEmptyClicked(){
  double radius = fillBlurRadius->value();
  int iter = fillIterations->value();
  qDebug("interpolate blur radius %f",radius);
  if(editor->editorView()->selectedImage()){
    qApp->setOverrideCursor(Qt::WaitCursor);
    editor->editorView()->selectedImage()->interpolateEmpty(radius,iter,editor->editorView()->selectedRegion(),fillBlurKernel->itemText(fillBlurKernel->currentIndex()));
    qApp->setOverrideCursor(Qt::ArrowCursor);
  }
}

void EditorTools::onCropClicked(){
  if(editor->editorView()->selectedImage()){
    editor->editorView()->selectedImage()->cropImage(editor->editorView()->selectedRegion());
  }  
}
