#include <QtGui>

#include "optionstree.h"
#include "treedelegate.h"
#include "configuration.h"

OptionsTree::OptionsTree(QWidget * parent)
  :QWidget(parent)
{
  setDefaultOptions(&global_options);
  createGUI();
}

OptionsTree::~OptionsTree(){
  int size = defaultOptions.size();
  for(int i = 0;i<size;i++){
    delete defaultOptions.at(i);
  }
}

void OptionsTree::setDefaultOptions(Options * opt){
  set_defaults(opt);
}

void OptionsTree::createGUI(){
  showOptionsCombo = new QComboBox(this);
  tree = new QTreeWidget(this);
  delegate = new ComboBoxDelegate(tree,showOptionsCombo);
  connect(delegate,SIGNAL(modelDataUpdated()),this,SLOT(rebuildTree()));
  tree->setEditTriggers(QAbstractItemView::AllEditTriggers);
  tree->setSelectionMode(QAbstractItemView::SingleSelection);
  tree->setItemDelegate(delegate);
  tree->setHeaderLabels(QStringList() << tr("Options")
			<< tr("Value"));
  tree->setWordWrap(true);
  tree->setAlternatingRowColors(true);
  tree->resize(150, 50);
  // This seems to fix an apparent bug in MacOSX which causes the file items to be too big
  //  tree->setUniformRowHeights(true);

  for (int i = 0; i < number_of_global_options; ++i) {
    // Don't show the root node 
    if(variable_metadata[i].id == Id_Root){
      continue;
    }
    // Variable is deprecated, don't show
    if(variable_metadata[i].variable_properties & deprecated){
      continue;
    }
    // Variable can't be set before run, don't show
    if((variable_metadata[i].variable_properties & isSettableBeforeRun) == 0){
      continue;
    }
    QTreeWidgetItem *nameItem = new QTreeWidgetItem(QTreeWidgetItem::Type);

    delegate->setItemFromMetadata(nameItem,&(variable_metadata[i]));

    nameItem->setData(1,Qt::UserRole,i);
    Q_ASSERT(variable_metadata[i].parent != 0);

    if(variable_metadata[i].parent->id == Id_Root){
      // we have a top level option
      tree->addTopLevelItem(nameItem);
    }else{
      const VariableMetadata * p = variable_metadata[i].parent;
      QString toMatch;
      if(QString(p->display_name).isEmpty()){
	toMatch = QString(p->variable_name);
      }else{
	toMatch = QString(p->display_name);
      }
      QList<QTreeWidgetItem *> matches = tree->findItems(toMatch,
							 Qt::MatchFixedString|
							 Qt::MatchRecursive|
							 Qt::MatchWrap,0);
      Q_ASSERT(!matches.isEmpty());	
      QTreeWidgetItem *parent = matches.first();
      Q_ASSERT(parent != 0);	
      parent->addChild(nameItem);
    }
    if(variable_metadata[i].variable_properties & advanced){	
      advancedOptions.append(nameItem);
    }
    if(variable_metadata[i].variable_properties & experimental){	
      experimentalOptions.append(nameItem);
    }
    allOptions.append(nameItem);
    defaultOptions.append(nameItem->clone());
  }
  tree->sortItems(0,Qt::AscendingOrder);
  tree->expandAll();
  tree->resizeColumnToContents(0);
  tree->resizeColumnToContents(1);
  tree->collapseAll();
  //    tree->horizontalHeader()->resizeSection(1, 150);
  
  QGridLayout *layout = new QGridLayout;
  layout->addWidget(tree, 0, 0,1,3);
  
  showOptionsCombo->setToolTip("Change the type of options you want to configure");
  showOptionsCombo->addItem("Basic");
  showOptionsCombo->addItem("Advanced");
  showOptionsCombo->addItem("In Testing");
  QPushButton * saveOptionsButton = new QPushButton(QIcon(":images/config_save.png"),tr("Save"),this);
  QPushButton * loadOptionsButton = new QPushButton(QIcon(":images/config_open.png"),tr("Load"),this);
  QPushButton * resetOptionButton = new QPushButton("Reset Option",this);
  QPushButton * resetAllOptionsButton = new QPushButton("Reset All",this);
  connect(saveOptionsButton, SIGNAL(clicked()), this, SLOT(saveOptions()));
  connect(loadOptionsButton, SIGNAL(clicked()), this, SLOT(loadOptions()));
  connect(showOptionsCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(rebuildTree()));
  connect(resetOptionButton, SIGNAL(clicked()), this, SLOT(resetSelectedOption()));
  connect(resetAllOptionsButton, SIGNAL(clicked()), this, SLOT(resetAllOptions()));
  layout->addWidget(showOptionsCombo, 1, 0);
  layout->addWidget(saveOptionsButton, 1, 1);
  layout->addWidget(loadOptionsButton, 1, 2);
  layout->addWidget(resetOptionButton, 2, 0);
  layout->addWidget(resetAllOptionsButton, 2, 1);
  
  setLayout(layout);
  
  setWindowTitle(tr("Color Editor Factory"));
  //    tree->resize(1000,1000);
  //    resize(sizeHint());
  resize(tree->columnWidth(0)+tree->columnWidth(1)+50,500);
  rebuildTree();
}

void OptionsTree::showAdvancedOptions(bool ){
  //  showAdvancedOptionsToggled = show;
  rebuildTree();
}

void OptionsTree::saveOptions(){
  QString file = QFileDialog::getSaveFileName (this,tr("Save Configuration File"),"uwrapc.conf", tr("Configuration (*.conf)"));
  if(file.isEmpty()){
    return;
  }
  write_options_file(file.toAscii().constData());
}

void OptionsTree::resetSelectedOption(){
  Options local;
  setDefaultOptions(&local);
  QList<QTreeWidgetItem *> items = tree->selectedItems();
  if(!items.size()){
    // No selected item
    return;
  }
  QTreeWidgetItem * item = items.first();  
  QTreeWidgetItem * def = NULL;
  int index = item->data(1,Qt::UserRole).toInt();
  for(int i = 0;i<defaultOptions.size();i++){
    int index2 = defaultOptions[i]->data(1,Qt::UserRole).toInt();
    if(index2 == index){
      def = defaultOptions[i];
    }
  }
  Q_ASSERT(def != NULL);
  delegate->setMetadataFromItem(def);  
  VariableMetadata * vm = &(variable_metadata[index]);
  delegate->setItemFromMetadata(item,vm);
  rebuildTree();
}

void OptionsTree::resetAllOptions(){
  if(QMessageBox::warning(this,"Reset All",
			   "Are you sure you want to reset all "
			   "options to their default values?",
			   QMessageBox::Yes|QMessageBox::No) 
     == QMessageBox::No){
    return;
  }
  setDefaultOptions(&global_options);
  for(int i = 0;i<allOptions.size();i++){
    int index = allOptions[i]->data(1,Qt::UserRole).toInt();
    VariableMetadata * vm = &(variable_metadata[index]);    
    delegate->setItemFromMetadata(allOptions[i],vm);
  }
  rebuildTree();
}

void OptionsTree::rebuildTree(){
  Options * opt = &global_options;
  for(int i = 0;i<allOptions.size();i++){
    int index = allOptions[i]->data(1,Qt::UserRole).toInt();
    VariableMetadata * vm = &(variable_metadata[index]);

    //    delegate->setItemFromMetadata(allOptions[i],vm);

    int hidden_flag = 0;
    if(vm->dependencies){ 
      if(vm->dependencies(opt) == 0){
	hidden_flag = 1;
      }
    }
    if((vm->variable_properties & advanced) && showOptionsCombo->currentIndex() < 1){
      //    if(vm->variable_properties & advanced && showAdvancedOptionsToggled == false){
      hidden_flag = 1;
    }
    if((vm->variable_properties & experimental) && showOptionsCombo->currentIndex() < 2){
      //    if(vm->variable_properties & experimental){
      hidden_flag = 1;
    }
    if(hidden_flag){
      allOptions[i]->setHidden(true);
    }else{
      allOptions[i]->setHidden(false);
    }
  }

  emit optionsTreeUpdated(opt);
}

void OptionsTree::loadOptions(){
  QString file =  QFileDialog::getOpenFileName (this,tr("Load Configuration File"),QString(),tr("Configuration (*.conf)"));
  if(file.isEmpty()){
    return;
  }  
  // Warning! Leads to memory leaks
  // I have to fix all this metadata mess 
  // and remove the global_options thing
  read_options_file(file.toAscii().data());
  QList<QTreeWidgetItem *> items = tree->findItems("*",Qt::MatchWildcard|Qt::MatchRecursive|Qt::MatchWrap,0);
  int size = items.size();
  for(int i = 0;i<size;i++){
    int idx = items.at(i)->data(1,Qt::UserRole).toInt();
    if(idx >= 0){
      delegate->setItemFromMetadata(items.at(i),&(variable_metadata[idx]));
    }
  }
  rebuildTree();
}

