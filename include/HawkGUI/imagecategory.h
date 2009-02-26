#ifndef _IMAGECATEGORY_H_
#define _IMAGECATEGORY_H_ 1

#include <QString>
#include <QRegExp>
#include <QList>


class ImageCategory{
 public:
  ImageCategory(QString name,QString id, bool iterationDependent = true);
  bool includes(const QString filename) const;
  QString getName() const;
  QString getIdentifier() const;
  QString fileNameFromIteration(QString iteration);
  static QString translateFilename(const QString filename, const ImageCategory * from, const ImageCategory * to);
  static ImageCategory * getFileCategory(const QString filename);
  static QString getFileIteration(const QString filename);
  static QString setFileIteration(const QString filename, QString iteration);
 private:
  QString getIteration(QString file);
  QRegExp regExp;
  QString name;
  QString identifier;
  static QList <ImageCategory *> categories;
  bool iterationSuffix;
};
#endif
