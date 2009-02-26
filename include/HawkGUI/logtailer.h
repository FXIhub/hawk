#ifndef _LOGTAILER_H_
#define _LOGTAILER_H_ 1

#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>

class LogTailer: public QFileSystemWatcher
{
  Q_OBJECT
 public:
  LogTailer(QObject * parent);
  void tailLogFile(QString path);
 public slots:
  void readLine(QString path);
 signals:
  void dataLineRead(QList<double> data);
  void headerRead(QString title, int column);
  private slots:
  void tryAddingPath();
 private:
  void parseLine(QString line);
  QFile log;
  qint64 pos;
  QTextStream reader;
  QString pathToAdd;
};

#endif
