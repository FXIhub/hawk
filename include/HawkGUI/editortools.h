#ifndef _EDITORTOOLS_H_
#define _EDITORTOOLS_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGroupBox>

class EditorWorkspace;
class QStackedLayout;

class EditorTools: public QGroupBox
{
  Q_OBJECT
    public:
  EditorTools(EditorWorkspace * parent);
 private slots:
  void onMathEdit();
  void onFilterClicked();
  void onPointerClicked();
  void onBullseyeClicked();
  void onDropClicked();
  void onSelectionClicked();
  void onLineoutClicked();
 private:
  EditorWorkspace * editor;
  QWidget * dropToolOptions;
  QWidget * filterToolOptions;
  QWidget * selectionToolOptions;
  QWidget * toolOptions;
  QStackedLayout * toolOptionsLayout;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
