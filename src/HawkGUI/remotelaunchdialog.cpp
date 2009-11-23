#include "remotelaunchdialog.h"
#include "rpcdefaultport.h"
#include <QtGui>
#include <QTcpSocket>

RemoteLaunchDialog::RemoteLaunchDialog(QWidget * parent, Qt::WindowFlags f)
  :QDialog(parent,f)
{
  QSettings settings;


  connect(this,SIGNAL(accepted()),this,SLOT(saveSettings()));
  QHBoxLayout * hbox = new QHBoxLayout();
  setLayout(hbox);
  QVBoxLayout * vbox = new QVBoxLayout();
  hbox->addLayout(vbox);

  vbox->addWidget(new QLabel("Select profile:"));
  m_profileList = new QListWidget(this);
  m_profileList->setResizeMode(QListView::Adjust);

  connect(m_profileList,SIGNAL(itemSelectionChanged()),this,SLOT(selectedProfileChanged()));
  vbox->addWidget(m_profileList);
  /* Add profiles from settings */
  m_profiles = settings.value("RemoteLaunchDialog/profileList").toStringList();

  QHBoxLayout * hbox2 = new QHBoxLayout();
  vbox->addLayout(hbox2);
  QPushButton * button = new QPushButton(QIcon(":/images/edit_add.png"),"");
  button->setAutoDefault(false);
  button->setIconSize(QSize(16,16));
  connect(button,SIGNAL(clicked()),this,SLOT(addProfile()));

  hbox2->addWidget(button);
  //  hbox2->addStretch();
  button = new QPushButton(QIcon(":/images/edit_remove.png"),"");
  button->setAutoDefault(false);
  button->setIconSize(QSize(16,16));
  connect(button,SIGNAL(clicked()),this,SLOT(removeProfile()));
  hbox2->addWidget(button);

  vbox = new QVBoxLayout();
  hbox->addLayout(vbox);
  QGroupBox * groupBox = new QGroupBox("Hosts settings",this);
  QGridLayout * grid = new QGridLayout(groupBox);
  grid->addWidget(new QLabel("Remote host:",groupBox),0,0);

  m_remoteHostEdit = new QLineEdit("",groupBox);
  connect(m_remoteHostEdit,SIGNAL(editingFinished()),m_profileList,SLOT(setFocus()));

  connect(m_remoteHostEdit,SIGNAL(editingFinished()),this,SLOT(remoteHostChanged()));
  grid->addWidget(m_remoteHostEdit,0,1);
  grid->addWidget(new QLabel("Remote port:"),1,0);

  m_remotePort = new QSpinBox(groupBox);
  m_remotePort->setRange(1,65535);
  connect(m_remotePort,SIGNAL(valueChanged(int)),this,SLOT(remotePortChanged(int)));

  grid->addWidget(m_remotePort,1,1);
  grid->addWidget(new QLabel("Local host:",groupBox),2,0);

  m_localHostEdit = new QLineEdit("",groupBox);
  connect(m_localHostEdit,SIGNAL(editingFinished()),this,SLOT(localHostChanged()));
  connect(m_localHostEdit,SIGNAL(editingFinished()),m_profileList,SLOT(setFocus()));

  grid->addWidget(m_localHostEdit,2,1);
  grid->addWidget(new QLabel("Auto local port:",groupBox),3,0);

  m_autoLocalPort = new QCheckBox(groupBox);
  connect(m_autoLocalPort,SIGNAL(toggled(bool)),this,SLOT(autoLocalPortToggled(bool)));

  grid->addWidget(m_autoLocalPort,3,1);
  localPortLabel = new QLabel("Local port:",groupBox);
  grid->addWidget(localPortLabel,4,0);
  m_localPort = new QSpinBox(groupBox);
  m_localPort->setRange(1,65535);

  connect(m_localPort,SIGNAL(valueChanged(int)),this,SLOT(localPortChanged(int)));

  grid->addWidget(m_localPort,4,1);


  vbox->addWidget(groupBox);
  groupBox = new QGroupBox("SSH settings",this);
  grid = new QGridLayout(groupBox);
  grid->addWidget(new QLabel("Local path:",groupBox),0,0);
  m_sshPathEdit = new QLineEdit("",groupBox);
  connect(m_sshPathEdit,SIGNAL(editingFinished()),m_profileList,SLOT(setFocus()));
  connect(m_sshPathEdit,SIGNAL(editingFinished()),this,SLOT(sshPathChanged()));
  grid->addWidget(m_sshPathEdit,0,1);
  vbox->addWidget(groupBox);

  groupBox = new QGroupBox("uwrapc settings",this);
  grid = new QGridLayout(groupBox);
  grid->addWidget(new QLabel("Remote path:",groupBox),0,0);
  m_uwrapcPathEdit = new QLineEdit("",groupBox);
  connect(m_uwrapcPathEdit,SIGNAL(editingFinished()),m_profileList,SLOT(setFocus()));
  connect(m_uwrapcPathEdit,SIGNAL(editingFinished()),this,SLOT(uwrapcPathChanged()));
  grid->addWidget(m_uwrapcPathEdit,0,1);
  vbox->addWidget(groupBox);

  hbox2 = new QHBoxLayout();
  vbox->addLayout(hbox2);

  hbox2->addStretch();
  button = new QPushButton("OK");  
  button->setAutoDefault(false);
  hbox2->addWidget(button);
  connect(button,SIGNAL(clicked()),this,SLOT(checkSettings()));


  button = new QPushButton("Cancel");
  button->setAutoDefault(false);
  hbox2->addWidget(button);
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));

  for(int i = 0;i<m_profiles.size();i++){
    m_remoteHostMap.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/remoteHost").toString());
    m_localHostMap.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/localHost").toString());
    m_remotePortMap.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/remotePort").toInt());
    m_localPortMap.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/localPort").toInt());
    m_autoLocalPortMap.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/autoLocalPort").toBool());
    m_sshPath.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/sshPath").toString());
    m_uwrapcPath.insert(m_profiles[i],settings.value("RemoteLaunchDialog/"+m_profiles[i]+"/uwrapcPath").toString());

    m_profileList->addItem(m_profiles[i]);      
    if(m_profiles[i] == settings.value("RemoteLaunchDialog/selectedProfile").toString()){
      m_profileList->setCurrentRow(m_profileList->count()-1);
    }
  }  
}

void RemoteLaunchDialog::selectedProfileChanged(){
  qDebug("RemoteLaunchDialog: Selected profile changed");

  QString selectedProfile = m_profileList->currentItem()->text();
  
  QString host = m_remoteHostMap.value(selectedProfile);
  m_remoteHostEdit->setText(host);

  host = m_localHostMap.value(selectedProfile);
  m_localHostEdit->setText(host);

  bool autoPort = m_autoLocalPortMap.value(selectedProfile);
  m_autoLocalPort->setChecked(autoPort);

  int port = m_remotePortMap.value(selectedProfile);
  m_remotePort->setValue(port);
  m_sshPathEdit->setText(m_sshPath.value(selectedProfile));
  m_uwrapcPathEdit->setText(m_uwrapcPath.value(selectedProfile));

  port = m_localPortMap.value(selectedProfile);
  m_localPort->setValue(port);
  if(autoPort){
    m_localPort->setEnabled(false);
    localPortLabel->setEnabled(false);    
  } 
}

void RemoteLaunchDialog::remoteHostChanged(){
  qDebug("RemoteLaunchDialog: Remote host changed");
  QString selectedProfile = m_profileList->currentItem()->text();
  m_remoteHostMap.insert(selectedProfile,m_remoteHostEdit->text());
}

void RemoteLaunchDialog::remotePortChanged(int port){
  qDebug("RemoteLaunchDialog: Remote port changed");
  QString selectedProfile = m_profileList->currentItem()->text();
  m_remotePortMap.insert(selectedProfile,port);

}

void RemoteLaunchDialog::localHostChanged(){
  QString selectedProfile = m_profileList->currentItem()->text();
  m_localHostMap.insert(selectedProfile,m_localHostEdit->text());
}

void RemoteLaunchDialog::localPortChanged(int port){
  QString selectedProfile = m_profileList->currentItem()->text();
  m_localPortMap.insert(selectedProfile,port);
}

void RemoteLaunchDialog::addProfile(){
  bool ok;
  QString text = QInputDialog::getText(this, tr("Add Profile"),
				       tr("Profile name:"), QLineEdit::Normal,
				       "custom", &ok);
  if(!ok){
    return;
  }
  if (text.isEmpty()){
    return;
  }
  if(!m_profileList->findItems(text,Qt::MatchFixedString).isEmpty()){
    QMessageBox::critical(this, tr("Add Profile"),
			  tr("Profile names must be unique.\n"),QMessageBox::Ok);
    return;
  }
  m_remotePortMap.insert(text,22);
  m_localPortMap.insert(text,rpcDefaultPort);
  m_localHostMap.insert(text,"localhost");
  m_remoteHostMap.insert(text,"localhost");
  m_sshPath.insert(text,"/usr/bin/ssh");
  m_uwrapcPath.insert(text,"/usr/bin/uwrapc");
  m_autoLocalPortMap.insert(text,true);
  m_profileList->addItem(text);
  m_profiles << text;
}

void RemoteLaunchDialog::removeProfile(){
  if(m_profileList->currentItem()->text() == "default"){
   QMessageBox::critical(this, tr("Remove Profile"),
			 tr("Can't remove the default profile.\n"),QMessageBox::Ok);
   return;     
  }
  m_profiles.removeAll(m_profileList->currentItem()->text());
  delete m_profileList->takeItem(m_profileList->currentRow());
}

void RemoteLaunchDialog::autoLocalPortToggled(bool checked){
  QString selectedProfile = m_profileList->currentItem()->text();
  m_localPort->setEnabled(!checked);
  localPortLabel->setEnabled(!checked);    
  m_autoLocalPortMap.insert(selectedProfile,checked);
}

void RemoteLaunchDialog::sshPathChanged(){
  QString selectedProfile = m_profileList->currentItem()->text();
  m_sshPath.insert(selectedProfile,m_sshPathEdit->text());
}

void RemoteLaunchDialog::uwrapcPathChanged(){
  QString selectedProfile = m_profileList->currentItem()->text();
  m_uwrapcPath.insert(selectedProfile,m_uwrapcPathEdit->text());

}

void RemoteLaunchDialog::saveSettings(){
  QSettings settings;
  qDebug("RemoteLaunchDialog: Saving settings");
  QString selectedProfile = m_profileList->currentItem()->text();
  settings.setValue("RemoteLaunchDialog/profileList",m_profiles);
  for(int i = 0;i<m_profiles.size();i++){
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/remoteHost", m_remoteHostMap.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/localHost", m_localHostMap.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/remotePort", m_remotePortMap.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/localPort", m_localPortMap.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/autoLocalPort", m_autoLocalPortMap.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/sshPath", m_sshPath.value(m_profiles[i]));
    settings.setValue("RemoteLaunchDialog/"+m_profiles[i]+"/uwrapcPath", m_uwrapcPath.value(m_profiles[i]));

    settings.setValue("RemoteLaunchDialog/selectedProfile",selectedProfile);
  }  
}

void RemoteLaunchDialog::checkSettings(){
  QSettings settings;
  bool settingsOK = true;
  QString selectedProfile = m_profileList->currentItem()->text();
  QString host = m_remoteHostMap.value(selectedProfile);
  int port = m_remotePortMap.value(selectedProfile);
  if(isHostAlive(host,port) == false){
    QMessageBox::warning(this, tr("HawkGUI"),
			 tr("Cannot connect to %1:%2.\n").arg(host).arg(port),
			 QMessageBox::Ok);
    settingsOK = false;
  }
  host = m_localHostMap.value(selectedProfile);
  if(m_autoLocalPortMap.value(selectedProfile)){
    port = settings.value("RPCServer/serverPort").toInt();
  }else{
    port = m_localPortMap.value(selectedProfile);
  }
  if(isHostAlive(host,port) == false){
    QMessageBox::warning(this, tr("HawkGUI"),
			  tr("Cannot connect to %1:%2.\n").arg(host).arg(port),
			  QMessageBox::Ok);
    settingsOK = false;
  }
  if(!QFile::exists(m_sshPath.value(selectedProfile))){
    QMessageBox::warning(this, tr("HawkGUI"),
			  tr("Can't find ssh executable in the specified path"),
			  QMessageBox::Ok);
    settingsOK = false;    
  }
  if(!settingsOK){
    int ret = QMessageBox::warning(this, tr("HawkGUI"),
				   tr("Some of the settings specified in the active profile seem to be invalid."
				      " Are you sure you want to save these settings?"),
				   QMessageBox::Save | QMessageBox::Cancel,
				   QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel){
      return;
    }
  }
  accept();
}

bool RemoteLaunchDialog::isHostAlive(QString host, int port, int msecTimeout){
  QTcpSocket socket(this);
  socket.connectToHost(host,port);
  bool alive = socket.waitForConnected(msecTimeout);    
  if(!alive){
    qDebug("RemoteLaunchDialog: %s",socket.errorString().toAscii().constData());
  }
  return alive;
}

int RemoteLaunchDialog::getLocalPortNumber(){
  QSettings settings;
  QString selectedProfile = settings.value("RemoteLaunchDialog/selectedProfile").toString();
  int port = settings.value("RemoteLaunchDialog/"+selectedProfile+"/localPort").toInt();
  bool autoPort = settings.value("RemoteLaunchDialog/"+selectedProfile+"/autoLocalPort").toBool();
  if(autoPort){
    port = settings.value("RPCServer/serverPort").toInt();
  }
  return port;
}
