#include <QtGui>

#include "addconstraintdialog.h"
#include "stitcherview.h"
#include "imageitem.h"


Q_DECLARE_METATYPE(ImageItem *);

AddConstraintDialog::AddConstraintDialog(StitcherView * v,QWidget * parent,Qt::WindowFlags f)
  :QDialog(parent,f)
{
  view = v;
  QVBoxLayout * vbox = new QVBoxLayout(this);
  QWidget * top = new QWidget(this);
  vbox->addWidget(top);
  QWidget * bottom = new QWidget(this);
  vbox->addWidget(bottom);
  QHBoxLayout * hbox = new QHBoxLayout(top);
  QWidget * left = new QWidget(top);
  hbox->addWidget(left);
  QVBoxLayout * leftVbox = new QVBoxLayout(left);
  QLabel * l = new QLabel("All Control Points",left);
  leftVbox->addWidget(l);
  l->setAlignment(Qt::AlignHCenter);
  leftList = new QListWidget(left);
  leftList->setAlternatingRowColors(true);
  leftList->setSelectionMode(QAbstractItemView::SingleSelection);
  fillAllPointsList();
  leftVbox->addWidget(leftList);
  QWidget * center = new QWidget(top);
  hbox->addWidget(center);
  QVBoxLayout * centerVbox = new QVBoxLayout(center);
  QPushButton * add = new QPushButton("Add",center);
  connect(add,SIGNAL(clicked()),this,SLOT(onAddClicked()));
  centerVbox->addWidget(add);
  QPushButton * del = new QPushButton("Delete",center);
  connect(del,SIGNAL(clicked()),this,SLOT(onDeleteClicked()));
  centerVbox->addWidget(del);
  QWidget * right = new QWidget(top);
  hbox->addWidget(right);
  QVBoxLayout * rightVbox = new QVBoxLayout(right);
  l = new QLabel("Constraining Points",right);
  l->setAlignment(Qt::AlignHCenter);
  rightVbox->addWidget(l);
  rightList = new QListWidget(right);
  rightList->setAlternatingRowColors(true);
  rightList->setSelectionMode(QAbstractItemView::SingleSelection);

  rightVbox->addWidget(rightList);
  
  QHBoxLayout * bottomHbox = new QHBoxLayout(bottom);
  QGroupBox * typeBox = new QGroupBox("Constraint Type",bottom);
  bottomHbox->addWidget(typeBox);
  
  QVBoxLayout * typeVBox = new QVBoxLayout(typeBox);
  line = new QRadioButton("Radial Line",typeBox);
  line->setChecked(true);
  typeVBox->addWidget(line);
  circle = new QRadioButton("Centered Circle",typeBox);
  typeVBox->addWidget(circle);

  QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  bottomHbox->addWidget(buttonBox);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  
}


void AddConstraintDialog::fillAllPointsList(){
  QList<QGraphicsItem *> it = view->items();
  for(int i = 0; i < it.size(); i++){
    if(ImageItem * item = qgraphicsitem_cast<ImageItem *>(it[i])){
      QString id = item->identifier();
      QList<QPointF> cp =  item->getControlPoints();
      for(int j = 0;j<cp.size();j++){
	QListWidgetItem * it = new QListWidgetItem(id + "." + QString::number(j+1),leftList,0);
	it->setData(Qt::UserRole,j);
	it->setData(Qt::UserRole+1,QVariant::fromValue(item));
	leftList->addItem(it);
      }
    }
  }
  leftList->sortItems(Qt::AscendingOrder);
}

void AddConstraintDialog::onAddClicked(){
  qDebug("Here");
  if(leftList->currentItem()){
    QListWidgetItem * it = leftList->currentItem();
    leftList->takeItem(leftList->row(it));
    rightList->addItem(it);
  }
  leftList->sortItems(Qt::AscendingOrder);
  rightList->sortItems(Qt::AscendingOrder);
}

void AddConstraintDialog::onDeleteClicked(){
  if(rightList->currentItem()){
    QListWidgetItem * it = rightList->currentItem();
    rightList->takeItem(rightList->row(it));
    leftList->addItem(it);
  }
  leftList->sortItems(Qt::AscendingOrder);
  rightList->sortItems(Qt::AscendingOrder);
}

GeometryConstraintType AddConstraintDialog::constraintType(){
  if(circle->isChecked()){
    return CircleConstraint;
  }
  if(line->isChecked()){
    return RadialLineConstraint;
  }
  return RadialLineConstraint;
}
QList<QPair<int,ImageItem *> > AddConstraintDialog::selectedPoints(){
  QList<QPair<int,ImageItem *> > ret;
  for(int i = 0;i<rightList->count();i++){
    QListWidgetItem * it = rightList->item(i);
    int p =  it->data(Qt::UserRole).value<int>();
    ImageItem * item =  it->data(Qt::UserRole+1).value<ImageItem *>();
    if(!item){
      abort();
    }
    ret.append(QPair<int,ImageItem *>(p,item));
  }
  return ret;
}
