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
  connect(bullseye,SIGNAL(clicked(bool)),this,SLOT(onBullseyeClicked()));
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
  lineout->setToolTip(tr("Trace plot lineout"));
  lineout->setIconSize(iconSize);
  lineout->setCheckable(true);
  lineout->setAutoExclusive(true);
  connect(lineout,SIGNAL(clicked(bool)),this,SLOT(onLineoutClicked()));
  layout->addWidget(lineout,0,6);

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
}


void EditorTools::onPointerClicked(){
  editor->editorView()->setDefaultMode();
  toolOptions->hide();
}

void EditorTools::onBullseyeClicked(){
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
    Image * image_list[2] = {0,0};
    if(editor->editorView()->imageItem() && editor->editorView()->imageItem()->getImage()){
      image_list[0] = editor->editorView()->imageItem()->getImage();
    }
    qDebug("Got formula %s\n",text.toAscii().data());
    Math_Output * out = evaluate_math_expression(text.toAscii().data(),image_list);
    if(out->type == MathOutputImage){
      /* We got a new image */
      ImageItem * item = new ImageItem(out->image,QString());
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

