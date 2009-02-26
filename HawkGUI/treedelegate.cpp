/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    delegate.cpp

    A delegate that allows the user to change integer values from the model
    using a spin box widget.
*/

#include <QtGui>

#include "treedelegate.h"
#include "mapeditordialog.h"


ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex & index ) const
{
  if(index.column() == 0){
    return NULL;
  }
  int i = index.model()->data(index, Qt::UserRole).toInt();
  VariableMetadata * md =  &(variable_metadata[i]);
  
  QWidget * ret;
  if(md->variable_type == Type_MultipleChoice){
    QComboBox *editor = new QComboBox(parent);
    QStringList options;
    for(int i = 0;md->list_valid_names[i] != 0;i++){
      options << md->list_valid_names[i];
    }
    for (int i = 0; i < options.size(); ++i) {
      editor->insertItem(i, options[i]);
    }
    ret = editor;
  }else if(md->variable_type == Type_Real){    
    if(md->variable_properties & withSpecialValue){
      QComboBox * editor = new QComboBox(parent);
      QDoubleValidator * sv = new QDoubleValidator(editor);
      sv->setNotation(QDoubleValidator::ScientificNotation);
      editor->setEditable(true);
      editor->setValidator(sv);
      editor->addItem(md->list_valid_names[0]);
      editor->setDuplicatesEnabled(false);
      ret = editor;
    }else{
      QLineEdit * editor = new QLineEdit(parent);
      QDoubleValidator * sv = new QDoubleValidator(editor);
      sv->setNotation(QDoubleValidator::ScientificNotation);
      editor->setValidator(sv);
      real v = *((real *)md->variable_address);
      editor->setText(QString::number(v));
      ret = editor;
    }
  }else if(md->variable_type == Type_Int){    
    if(md->variable_properties & withSpecialValue){
      QComboBox * editor = new QComboBox(parent);
      QIntValidator * sv = new QIntValidator(editor);
      editor->setEditable(true);
      editor->setValidator(sv);
      editor->addItem(md->list_valid_names[0]);
      editor->setDuplicatesEnabled(false);
      ret = editor;
    }else{
      QSpinBox * editor = new QSpinBox(parent);
      editor->setRange(INT_MIN,INT_MAX);
      int v = *((int *)md->variable_address);
      editor->setValue(v);
      ret = editor;
    }
  }else if(md->variable_type == Type_Bool){    
    QComboBox *editor = new QComboBox(parent);
    QStringList options;
    options << "true";
    options << "false";
    for (int i = 0; i < options.size(); ++i) {
      editor->insertItem(i, options[i]);
    }
    ret = editor;
  }else if(md->variable_type == Type_Filename){  
    QFileInfo fi = QFileInfo((char *)md->variable_address);
    QFileDialog * editor = new QFileDialog(NULL,Qt::Dialog);
    editor->setDirectory(fi.path());
    editor->setWindowTitle(md->variable_name);
    editor->setModal(true);
    connect(editor,SIGNAL(finished(int)),this, SLOT(commitAndCloseFileEditor(int)));
    ret = editor;
  }else if(md->variable_type == Type_Directory_Name){  
    QFileDialog * editor = new QFileDialog(NULL,Qt::Dialog);
    editor->setFileMode(QFileDialog::DirectoryOnly);
    connect(editor,SIGNAL(finished(int)),this, SLOT(commitAndCloseFileEditor(int)));
    editor->setModal(true);
    editor->setWindowTitle(md->variable_name);
    ret = editor;
  }else if(md->variable_type == Type_Map_Real){  
    MapEditorDialog * editor = new MapEditorDialog(parent);
    editor->setModal(true);
    connect(editor,SIGNAL(finished(int)),this, SLOT(commitAndCloseMapEditor(int)));
    ret = editor;
  }else{
    ret = NULL;
  }
  return ret;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  int i = index.model()->data(index, Qt::UserRole).toInt();
  VariableMetadata * md =  &(variable_metadata[i]);
  if(md->variable_type == Type_MultipleChoice){
    QComboBox *comboBox = static_cast<QComboBox*>(editor);  
    QString option = index.model()->data(index, Qt::DisplayRole).toString();
    comboBox->setCurrentIndex(comboBox->findData(option, int(Qt::DisplayRole)));
  }else if(md->variable_type == Type_Real){    
    if(md->variable_properties & withSpecialValue){
      QComboBox *comboBox = static_cast<QComboBox*>(editor);  
      QFont font = index.data(Qt::FontRole).value<QFont>();
      QString value = displayFromMetadata(md,font).toString();
      if(comboBox->findText(value) != -1){
	value = QString("");
      }
      comboBox->addItem(value);
      comboBox->setCurrentIndex(comboBox->findText(value));
      connect(comboBox,SIGNAL(activated(int)),this,SLOT(specialValueComboBoxActivated(int)));
    }else{
      double v = index.model()->data(index, Qt::DisplayRole).toDouble();
      QLineEdit * le = static_cast<QLineEdit*>(editor);  
      le->setText(QString::number(v));
    }
  }else if(md->variable_type == Type_Int){    
    if(md->variable_properties & withSpecialValue){
      QComboBox *comboBox = static_cast<QComboBox*>(editor);  
      QFont font = index.data(Qt::FontRole).value<QFont>();
      QString value = displayFromMetadata(md,font).toString();
      if(comboBox->findText(value) != -1){
	value = QString("");
      }
      comboBox->addItem(value);
      comboBox->setCurrentIndex(comboBox->findText(value));
      connect(comboBox,SIGNAL(activated(int)),this,SLOT(specialValueComboBoxActivated(int)));
    }else{
      int v = index.model()->data(index, Qt::DisplayRole).toInt();
      QSpinBox * sb = static_cast<QSpinBox*>(editor);  
      sb->setValue(v);
    }
  }else if(md->variable_type == Type_Bool){    
    QComboBox *comboBox = static_cast<QComboBox*>(editor);  
    QString option = index.model()->data(index, Qt::DisplayRole).toString();
    comboBox->setCurrentIndex(comboBox->findData(option, int(Qt::DisplayRole)));
  }else if(md->variable_type == Type_Filename){      
    QFileInfo fi = QFileInfo((char *)md->variable_address);
    QFileDialog * fe = static_cast<QFileDialog*>(editor);  
    fe->selectFile(fi.fileName());
    fe->resize(fe->sizeHint());
  }else if(md->variable_type == Type_Directory_Name){  
    QFileInfo fi = QFileInfo((char *)md->variable_address);
    fi.makeAbsolute();
    QFileDialog * fe = static_cast<QFileDialog*>(editor);  
    fe->setDirectory(fi.path());
    fe->resize(fe->sizeHint());
    fe->selectFile(fi.fileName());
  }else if(md->variable_type == Type_Map_Real){
    MapEditorDialog * me = static_cast<MapEditorDialog *>(editor);  
    sp_smap * map = *((sp_smap **)md->variable_address);
    sp_list * keys = sp_smap_get_keys(map);
    sp_list * values = sp_smap_get_values(map);
    QPolygonF points;
    for(unsigned int i = 0;i<sp_list_size(keys);i++){
      points << QPointF(sp_list_get(keys,i), sp_list_get(values,i));
    }
    me->setPoints(points);
    me->setXLabel("Iterations");
    me->setYLabel(md->display_name);
    me->resize(me->sizeHint());
    me->update();
  }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  int i = index.model()->data(index, Qt::UserRole).toInt();
  VariableMetadata * md =  &(variable_metadata[i]);
  if(md->variable_type == Type_MultipleChoice){
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString option = (comboBox->itemData(comboBox->currentIndex(), Qt::DisplayRole)).toString();    
    model->setData(index, option, Qt::DisplayRole);
  }else if(md->variable_type == Type_Real){    
    if(md->variable_properties & withSpecialValue){
      QComboBox *comboBox = static_cast<QComboBox*>(editor);
      QString option = (comboBox->itemData(comboBox->currentIndex(), Qt::DisplayRole)).toString();    
      model->setData(index, option, Qt::DisplayRole);
    }else{
      QLineEdit * le = static_cast<QLineEdit*>(editor);  
      real v = le->text().toDouble();
      model->setData(index,v,Qt::DisplayRole);
    }
  }else if(md->variable_type == Type_Int){    
    if(md->variable_properties & withSpecialValue){
      QComboBox *comboBox = static_cast<QComboBox*>(editor);
      QString option = (comboBox->itemData(comboBox->currentIndex(), Qt::DisplayRole)).toString();    
      model->setData(index, option, Qt::DisplayRole);
    }else{
      QSpinBox * sb = static_cast<QSpinBox*>(editor);  
      int v = sb->value();
      model->setData(index, v, Qt::DisplayRole);
    }
  }else if(md->variable_type == Type_Bool){
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString option = (comboBox->itemData(comboBox->currentIndex(), Qt::DisplayRole)).toString();    
    model->setData(index, option, Qt::DisplayRole);
  }else if(md->variable_type == Type_Filename){
    QFileDialog * fe = static_cast<QFileDialog*>(editor);  
    QStringList sf = fe->selectedFiles();
    if(fe->result() == QDialog::Accepted && sf.size()){
      QFileInfo fi(sf.first());
      model->setData(index, fi.fileName(), Qt::DisplayRole);
      model->setData(index, fi.absoluteFilePath(), Qt::ToolTipRole);
    }
  }else if(md->variable_type == Type_Directory_Name ){
    QFileDialog * fe = static_cast<QFileDialog*>(editor);  
    QStringList sf = fe->selectedFiles();
    if(fe->result() == QDialog::Accepted && sf.size()){
      QFileInfo fi(sf.first());
      fi.makeAbsolute();
      QString display =  fi.absoluteFilePath();
      QFontMetrics fm = QFontMetrics(index.data(Qt::FontRole).value<QFont>());
      QString elided = fm.elidedText(display,Qt::ElideLeft,200);
      model->setData(index, elided, Qt::DisplayRole);
      model->setData(index, fi.absoluteFilePath(), Qt::ToolTipRole);
    }
  }else if(md->variable_type == Type_Map_Real ){
    MapEditorDialog * me = static_cast<MapEditorDialog *>(editor);  
    if(me->result() != QDialog::Accepted){
      return;
    }
    sp_smap * map = *((sp_smap **)md->variable_address);
    QPolygonF points = me->getPoints();
    QFont font = index.data(Qt::FontRole).value<QFont>();
    sp_smap_clear(map);
    for(int i = 0;i<points.size();i++){
      sp_smap_insert(map,points[i].x(),points[i].y());
    }
    model->setData(index, displayFromMetadata(md,font), Qt::DisplayRole);
    model->setData(index, decorationFromMetadata(md,font), Qt::DecorationRole);
    model->setData(index, extraDataFromMetadata(md,font), Qt::UserRole+1);
    
  }
  setMetadataFromModel(index);
  emit modelDataUpdated();
}




void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  QItemDelegate::paint(painter,option,index);
}

QVariant ComboBoxDelegate::displayFromMetadata(const VariableMetadata * md,QFont font) const {
  if(md->variable_type == Type_String){
    return QVariant((char *)md->variable_address);
  }
  if(md->variable_type == Type_Filename){
    QFileInfo f = QFileInfo((char *)md->variable_address);
    if(f.fileName().isEmpty()){
      return QVariant("<none>");
    }
    return QVariant(f.fileName());
  }
  if(md->variable_type == Type_Directory_Name){
    QFileInfo f = QFileInfo((char *)md->variable_address);
    if(((char *)md->variable_address)[0] == 0){
      return QVariant("<none>");
    }
    QFontMetrics fm = QFontMetrics(font);
    QString elided = fm.elidedText(f.absoluteFilePath(),Qt::ElideLeft,200);
    return QVariant(elided);
  }
  if(md->variable_type == Type_Real){
    if(md->variable_properties & withSpecialValue){
      for(int i = 0;md->list_valid_names[i];i++){
	if(md->list_valid_values[i] == *((real *)md->variable_address)){
	  return QVariant(QString(md->list_valid_names[i]));
	}
      }
    }
    return QVariant(QString::number(*((real *)md->variable_address)));
  }
  if(md->variable_type == Type_Int){
    if(md->variable_properties & withSpecialValue){
      for(int i = 0;md->list_valid_names[i];i++){
	if(md->list_valid_values[i] == *((int *)md->variable_address)){
	  return QVariant(QString(md->list_valid_names[i]));
	}
      }
    }
    return QVariant(*((int *)md->variable_address));
  }
  if(md->variable_type == Type_MultipleChoice){    
    return QVariant(md->list_valid_names[*((int *)md->variable_address)]);
  }
  if(md->variable_type == Type_Bool){    
    int v = *((int *)md->variable_address);
    if(v){
      return QVariant(true);
    }else{
      return QVariant(false);
    }
  }
  if(md->variable_type == Type_Group){
    return QVariant("");
  }
  if(md->variable_type == Type_Image){
    return QVariant("Image");
  }
  if(md->variable_type == Type_Slice){
    return QVariant("Slice");
  }
  if(md->variable_type == Type_Vector_Real ||md->variable_type == Type_Vector_Int){
    sp_vector * v = (sp_vector *)md->variable_address;
    QString s = "{";
    for(unsigned int i = 0;i<sp_vector_size(v);i++){
      s += sp_vector_get(v,i);
      s += ",";
    }
    // remove the trailing comma 
    if(sp_vector_size(v)){
      s.chop(1);
    }
    s += "}";
    return QVariant(s);
  }
  if(md->variable_type == Type_Map_Real){
    sp_smap * map = *((sp_smap**)md->variable_address);
    sp_list * values = sp_smap_get_values(map);
    real first = sp_list_get(values,0);
    real last = sp_list_get(values,sp_list_size(values)-1);
    if(sp_list_size(values) > 1){
      return QVariant(QString::number(first,'g',2)+QString(" to ")+QString::number(last,'g',2));
    }else{
      return QVariant(QString::number(first,'g',2));
    }
  }
  return QVariant();
}

QVariant ComboBoxDelegate::decorationFromMetadata(const VariableMetadata * md,QFont ) const {
  if(md->variable_type == Type_Map_Real){
    sp_smap * map = *((sp_smap**)md->variable_address);
    sp_list * keys = sp_smap_get_keys(map);
    sp_list * values = sp_smap_get_values(map);
    int width = 60;
    int height = 20;
    if(sp_list_size(keys)){
      QPixmap plot(width,height);
      QPainter painter(&plot);   
      painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing,true);
      painter.fillRect(plot.rect(),QBrush(Qt::white));
      real v_max = sp_list_get(values,0);
      real v_min = sp_list_get(values,0);
      for(unsigned int i = 1;i<sp_list_size(values);i++){
	real v = sp_list_get(values,i);
	if(v > v_max){
	  v_max = v;
	}
	if(v < v_min){
	  v_min = v;
	}
      }
      real range = v_max - v_min;
      QList<real> tvalues;
      QList<real> tkeys;
      if(range){
	for(unsigned int i = 0;i<sp_list_size(values);i++){
	  tvalues.append((sp_list_get(values,i)-v_min)/range);
	}
	for(unsigned int i = 0;i<sp_list_size(keys);i++){
	  tkeys.append(sp_list_get(keys,i)/
		       (sp_list_get(keys,sp_list_size(keys)-1)));
	}
      }else{
	tvalues.append(0.5);
	tvalues.append(0.5);
	tkeys.append(0);
	tkeys.append(1);
      }

      QPainterPath myPath(QPointF(0,height));
      myPath.lineTo(QPointF(0,height*(1-tvalues[0])));
      myPath.lineTo(QPointF(width*(tkeys[0]),height*(1-tvalues[0])));
      for(int i = 0;i<tkeys.size()-1;i++){	
	QPointF c1(width*(tkeys[i]+tkeys[i+1])/2,height*(1-tvalues[i]));
	QPointF c2(width*((tkeys[i]+tkeys[i+1])/2),height*(1-tvalues[i+1]));
	QPointF endPoint(width*(tkeys[i+1]),height*(1-tvalues[i+1]));
	myPath.cubicTo(c1, c2, endPoint);
      }
      myPath.lineTo(QPointF(width,height));
      QLinearGradient grad = QLinearGradient(0,0,0,height);
      //      grad.setColorAt(0,QColor("#4F94CD"));
      grad.setColorAt(0,QColor("#B2DFEE"));
      grad.setColorAt(1,QColor("#26466D"));
      QBrush grad_brush(grad);      
      painter.fillPath(myPath,grad_brush);
      QPen p = painter.pen();
      p.setColor("#B2DFEE");
      painter.setPen(p);
      painter.drawPath(myPath);
      return QVariant(plot);	
    }
  }
  return QVariant();
}

void ComboBoxDelegate::commitAndCloseFileEditor(int r)
{
  QFileDialog *editor = qobject_cast<QFileDialog *>(sender());
  if(r){
    emit commitData(editor);
  }
  emit closeEditor(editor);
}


void ComboBoxDelegate::commitAndCloseMapEditor(int r)
{
  MapEditorDialog *editor = qobject_cast<MapEditorDialog *>(sender());
  if(r){
    emit commitData(editor);
  }
  emit closeEditor(editor);
}

void ComboBoxDelegate::specialValueComboBoxActivated(int index){
  QComboBox * cb = qobject_cast<QComboBox *>(sender());
  if(index == 0){
    cb->setEditable(false);
  }else{
    cb->setEditable(true);
  }
}

QVariant ComboBoxDelegate::tooltipFromMetadata(const VariableMetadata * vm,
					       QFont) const {
  if(vm->variable_type == Type_Filename){
    QFileInfo f = QFileInfo((char *)vm->variable_address);
    if(!QString((char *)vm->variable_address).isEmpty()){
      return f.absoluteFilePath();
    }
  }
  if(vm->variable_type == Type_Directory_Name){
    QFileInfo f = QFileInfo((char *)vm->variable_address);	
    return f.absoluteFilePath();
  }
  return QString("");
}


QVariant ComboBoxDelegate::extraDataFromMetadata(const VariableMetadata * vm,
					       QFont) const {
  if(vm->variable_type == Type_Map_Real ){
    sp_smap * map = *((sp_smap **)vm->variable_address);
    sp_list * keys = sp_smap_get_keys(map);
    sp_list * values = sp_smap_get_values(map);
    DoubleMap qmap;
    for(unsigned int i = 0;i<sp_list_size(keys);i++){
      qmap.insert(sp_list_get(keys,i), sp_list_get(values,i));
    }
    return qVariantFromValue(qmap);
  }
  return QVariant();
}

void ComboBoxDelegate::setItemFromMetadata(QTreeWidgetItem * item,
					   const VariableMetadata * vm){
  if(QString(vm->display_name).isEmpty()){
    item->setData(0,Qt::DisplayRole, vm->variable_name);
  }else{
    item->setData(0,Qt::DisplayRole, vm->display_name);
  }
  // The <p> </p> are there to make sure the text is interpreted as rich text
  item->setData(0,Qt::ToolTipRole,QString("<p>")+QString(vm->documentation)+QString("</p>"));
  QFont font = item->font(0);
  item->setData(1,Qt::DisplayRole, 
		displayFromMetadata(vm,font));
  item->setData(1,Qt::DecorationRole, 
		decorationFromMetadata(vm,font));
  item->setData(1,Qt::ToolTipRole,tooltipFromMetadata(vm,font));
  item->setData(1,Qt::UserRole+1,extraDataFromMetadata(vm,font));
  item->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}


void ComboBoxDelegate::setMetadataFromModel(const QModelIndex &index) const{
  int i = index.data(Qt::UserRole).toInt();
  VariableMetadata * md =  &(variable_metadata[i]);  
  if(md->variable_type == Type_MultipleChoice){
    QString option = index.data(Qt::DisplayRole).toString();    
    (*(int *)md->variable_address) = get_list_value_from_list_name(md,option.toLatin1().data());
  }else if(md->variable_type == Type_Real){    
    if(md->variable_properties & withSpecialValue){
      QString option = index.data(Qt::DisplayRole).toString();    
      bool ok;
      real value = option.toDouble(&ok);
      if(ok){
	(*(real *)md->variable_address) = value;
      }else{
	(*(real *)md->variable_address) = md->list_valid_values[0];
      }      
    }else{
      real v = index.data(Qt::DisplayRole).toDouble();    
      (*(real *)md->variable_address) = v;
    }
  }else if(md->variable_type == Type_Int){    
    if(md->variable_properties & withSpecialValue){
      QString option = index.data(Qt::DisplayRole).toString();    
      bool ok;
      int value = option.toInt(&ok);
      if(ok){
	(*(int *)md->variable_address) = value;
      }else{
	(*(int *)md->variable_address) = md->list_valid_values[0];
      }      
    }else{
      int v = index.data(Qt::DisplayRole).toInt();
      (*(int *)md->variable_address) = v;
    }
  }else if(md->variable_type == Type_Bool){
    QString option = index.data(Qt::DisplayRole).toString();    
    if(option == QString("true")){
      (*(int *)md->variable_address) = 1;
    }else{
      (*(int *)md->variable_address) = 0;
    }
  }else if(md->variable_type == Type_Filename){
    QString option = index.data(Qt::ToolTipRole).toString();
    strcpy((char *)md->variable_address,option.toLatin1().data());
  }else if(md->variable_type == Type_Directory_Name ){
    QString option = index.data(Qt::ToolTipRole).toString();
    strcpy((char *)md->variable_address,option.toLatin1().data());
  }else if(md->variable_type == Type_Map_Real ){
    DoubleMap qmap;
    sp_smap * map = *((sp_smap **)md->variable_address);
    qmap = qVariantValue<DoubleMap>(index.data(Qt::UserRole+1));
    sp_smap_clear(map);
    QList<double> values = qmap.values();
    QList<double> keys = qmap.keys();
    for(int i = 0;i<qmap.size();i++){
      sp_smap_insert(map,keys[i],values[i]);
    }
  }
}


void ComboBoxDelegate::setMetadataFromItem(const QTreeWidgetItem * item) const{
  int i = item->data(1,Qt::UserRole).toInt();
  VariableMetadata * md =  &(variable_metadata[i]);  
  if(md->variable_type == Type_MultipleChoice){
    QString option = item->data(1,Qt::DisplayRole).toString();    
    (*(int *)md->variable_address) = get_list_value_from_list_name(md,option.toLatin1().data());
  }else if(md->variable_type == Type_Real){    
    if(md->variable_properties & withSpecialValue){
      QString option = item->data(1,Qt::DisplayRole).toString();    
      bool ok;
      real value = option.toDouble(&ok);
      if(ok){
	(*(real *)md->variable_address) = value;
      }else{
	(*(real *)md->variable_address) = md->list_valid_values[0];
      }      
    }else{
      real v = item->data(1,Qt::DisplayRole).toDouble();    
      (*(real *)md->variable_address) = v;
    }
  }else if(md->variable_type == Type_Int){    
    if(md->variable_properties & withSpecialValue){
      QString option = item->data(1,Qt::DisplayRole).toString();    
      bool ok;
      int value = option.toInt(&ok);
      if(ok){
	(*(int *)md->variable_address) = value;
      }else{
	(*(int *)md->variable_address) = md->list_valid_values[0];
      }      
    }else{
      int v = item->data(1,Qt::DisplayRole).toInt();
      (*(int *)md->variable_address) = v;
    }
  }else if(md->variable_type == Type_Bool){
    QString option = item->data(1,Qt::DisplayRole).toString();    
    if(option == QString("true")){
      (*(int *)md->variable_address) = 1;
    }else{
      (*(int *)md->variable_address) = 0;
    }
  }else if(md->variable_type == Type_Filename){
    QString option = item->data(1,Qt::ToolTip).toString();
    strcpy((char *)md->variable_address,option.toLatin1().data());
  }else if(md->variable_type == Type_Directory_Name ){
    QString option = item->data(1,Qt::ToolTip).toString();
    strcpy((char *)md->variable_address,option.toLatin1().data());
  }else if(md->variable_type == Type_Map_Real ){
    DoubleMap qmap;
    sp_smap * map = *((sp_smap **)md->variable_address);
    qmap = qVariantValue<DoubleMap>(item->data(1,Qt::UserRole+1));
    sp_smap_clear(map);
    QList<double> values = qmap.values();
    QList<double> keys = qmap.keys();
    for(int i = 0;i<qmap.size();i++){
      sp_smap_insert(map,keys[i],values[i]);
    }
  }
}
