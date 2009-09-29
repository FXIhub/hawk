#ifndef _EDITORTOOLS_H_
#define _EDITORTOOLS_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

#include <QGroupBox>

class EditorWorkspace;
class QStackedLayout;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QListWidget;
class QComboBox;

class EditorTools: public QGroupBox
{
  Q_OBJECT
    public:
  EditorTools(EditorWorkspace * parent);
  enum SelectionMode{SelectionSet,SelectionUnite,SelectionSubtract};
  SelectionMode selectionMode();
 private slots:
  void onMathEdit();
  void onFilterClicked();
  void onPointerClicked();
  void onBullseyeToggled(bool checked);
  void onDropClicked();
  void onSelectionClicked();
  void onLineoutClicked();
  void onUndoClicked();
  void onRedoClicked();
  void onRemoveElectronicsClicked();
  void onRemoveVerticalLinesClicked();
  void onRemoveHorizontalLinesClicked();
  void setSelectionModeSet();
  void setSelectionModeUnite();
  void setSelectionModeSubtract();
  void onSelectByExpression();
  void onXcamMagicClicked();
  void onFillEmptyClicked();
  void onInterpolateEmptyClicked();
  void onCropClicked();
 private:
  EditorWorkspace * editor;
  QWidget * dropToolOptions;
  QWidget * filterToolOptions;
  QWidget * selectionToolOptions;
  QWidget * electronicsToolOptions;
  QWidget * fillEmptyToolOptions;
  QWidget * toolOptions;
  QLineEdit * selectExpression;
  QStackedLayout * toolOptionsLayout;
  SelectionMode _selectionMode;
  QDoubleSpinBox * fillBlurRadius;
  QComboBox * fillBlurKernel;
  QSpinBox * fillIterations;
};

#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
