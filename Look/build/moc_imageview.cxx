/****************************************************************************
** Meta object code from reading C++ file 'imageview.h'
**
** Created: Tue Apr 29 14:56:38 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../imageview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'imageview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_ImageView[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      27,   10,   10,   10, 0x05,
      45,   10,   10,   10, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_ImageView[] = {
    "ImageView\0\0centerChanged()\0beamstopChanged()\0"
    "vertLineSet()\0"
};

const QMetaObject ImageView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ImageView,
      qt_meta_data_ImageView, 0 }
};

const QMetaObject *ImageView::metaObject() const
{
    return &staticMetaObject;
}

void *ImageView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ImageView))
	return static_cast<void*>(const_cast< ImageView*>(this));
    return QWidget::qt_metacast(_clname);
}

int ImageView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: centerChanged(); break;
        case 1: beamstopChanged(); break;
        case 2: vertLineSet(); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void ImageView::centerChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ImageView::beamstopChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void ImageView::vertLineSet()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
