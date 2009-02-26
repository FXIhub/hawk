#ifndef _OUTPUTWATCHER_H_
#define _OUTPUTWATCHER_H_ 1

#include <QFileSystemWatcher>
#include <QThread>
#include <QMap>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QList>

class QTimer;
class ImageCategory;


//class OutputWatcher: public QFileSystemWatcher
class OutputWatcher: public QThread
{
  Q_OBJECT
 public:
  OutputWatcher(QString outputDir,QObject * parent,QList<ImageCategory *> * ic,int inc,QDateTime startTime);
  void stop();
  QList<QFileInfo> getOutputFiles();
  bool fileExists(QString file);
  QFileInfo getFileInfo(QString file);
  QFileInfo getNextFile(QString file);
  QFileInfo getPreviousFile(QString file);
  static QString incrementFilename(QString file, int increment);
  bool isFileValid(QString file);
 signals:
  void newOutput(QString type, QFileInfo fi,QFileInfo old);
  void initialOutput(QString type,QFileInfo fi);
  private slots:
  void checkForNewFiles();
 protected:
  void run();
 private:
  void setupQDir(QString path);
  int processFile(const QString file);
  QDir dir;
  QMap<QString,QFileInfo>newestFiles;
  QList<QFileInfo>outputFiles;
  QDateTime initTime;
  QTimer * pooler;
  QList<ImageCategory *> * imageCategories;
  int increment;
};
#endif
