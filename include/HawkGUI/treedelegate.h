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

#ifndef DELEGATE_H
#define DELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>
#include <QTreeWidget>
#include <QFont>
#include <QMap>
#include <QFileDialog>

#include "configuration.h"

class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ComboBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QVariant displayFromMetadata(const VariableMetadata * md,QFont font) const;
    QVariant decorationFromMetadata(const VariableMetadata * md,QFont font) const;
    QVariant tooltipFromMetadata(const VariableMetadata * md,QFont font) const;
    QVariant extraDataFromMetadata(const VariableMetadata * md,QFont font) const;    
    void setItemFromMetadata(QTreeWidgetItem * item, const VariableMetadata * vm);
    void setMetadataFromModel(const QModelIndex &index) const;
    void setMetadataFromItem(const QTreeWidgetItem * item) const;
 signals:
    void modelDataUpdated() const;
 private slots:
    void commitAndCloseFileEditor(int r);
    void commitAndCloseMapEditor(int r);
    void specialValueComboBoxActivated(int index);
 private:
    QMap<QFileDialog *,QString> editorResult;
    int fileEditorReturn;
    QString fileEditorValue;
};

typedef QMap<double,double> DoubleMap;

Q_DECLARE_METATYPE(DoubleMap);

#endif
