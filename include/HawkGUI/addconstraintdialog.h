#ifndef _ADD_CONSTRAINT_DIALOG_H_
#define _ADD_CONSTRAINT_DIALOG_H_ 1

#include <QDialog>
#include "geometry_constraints.h"


class StitcherView;
class QListWidget;
class ImageItem;
class QRadioButton;

class AddConstraintDialog: public QDialog
{
  Q_OBJECT
    public:
  AddConstraintDialog(StitcherView * view ,QWidget * parent = 0,Qt::WindowFlags f = 0);
 public:
  QList<QPair<int,ImageItem *> > selectedPoints();
  GeometryConstraintType constraintType();
  private slots:
  void onAddClicked();
  void onDeleteClicked();
 private:
  void fillAllPointsList();
  QListWidget * leftList;
  QListWidget * rightList;
  StitcherView * view;
  QRadioButton * circle;
  QRadioButton * line;
};

#endif
