#ifndef _REMOTE_LAUNCH_DIALOG_H_
#define _REMOTE_LAUNCH_DIALOG_H_ 1

#include <QDialog>
#include <QMap>

class QListWidget;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QLabel;

class RemoteLaunchDialog: public QDialog
{
  Q_OBJECT
    public:
  RemoteLaunchDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
  
  static int getLocalPortNumber();
 private slots:
  void selectedProfileChanged();
  void remoteHostChanged();
  void remotePortChanged(int);
  void localHostChanged();
  void localPortChanged(int);
  void addProfile();
  void removeProfile();
  void autoLocalPortToggled(bool checked);
  void sshPathChanged();
  void saveSettings();
  void checkSettings();
 private:
  bool isHostAlive(QString host, int port,int msecTimeout=1000);
  QListWidget * m_profileList;
  QLineEdit * m_sshPathEdit;
  QLineEdit * m_remoteHostEdit;
  QSpinBox * m_remotePort;
  QLineEdit * m_localHostEdit;
  QSpinBox * m_localPort;
  QCheckBox * m_autoLocalPort;
  QLabel * localPortLabel;
  QStringList m_profiles;
  QMap<QString,QString> m_remoteHostMap;
  QMap<QString,QString> m_localHostMap;
  QMap<QString,int> m_remotePortMap;
  QMap<QString,int> m_localPortMap;
  QMap<QString,int> m_autoLocalPortMap;
  QString m_sshPath;
};

#endif
