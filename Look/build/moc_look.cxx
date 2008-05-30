/****************************************************************************
** Meta object code from reading C++ file 'look.h'
**
** Created: Wed Apr 23 09:09:54 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../look.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'look.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Look[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x05,
      30,    5,    5,    5, 0x05,
      47,    5,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,    5,    5,    5, 0x0a,
      90,    5,    5,    5, 0x08,
     111,    5,    5,    5, 0x08,
     137,  131,    5,    5, 0x08,
     154,  131,    5,    5, 0x08,
     181,  131,    5,    5, 0x08,
     209,    5,    5,    5, 0x08,
     224,    5,    5,    5, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Look[] = {
    "Look\0\0backgroundChecked(bool)\0"
    "hitChecked(bool)\0imgFromListChanged(bool)\0"
    "loadAllComments()\0openImageFromTable()\0"
    "openImageFromList()\0value\0changeRange(int)\0"
    "changeBackgroundRange(int)\0"
    "changeBackgroundLevel(real)\0updateCenter()\0"
    "updateBeamstop()\0"
};

const QMetaObject Look::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Look,
      qt_meta_data_Look, 0 }
};

const QMetaObject *Look::metaObject() const
{
    return &staticMetaObject;
}

void *Look::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Look))
	return static_cast<void*>(const_cast< Look*>(this));
    return QWidget::qt_metacast(_clname);
}

int Look::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: backgroundChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: hitChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: imgFromListChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: loadAllComments(); break;
        case 4: openImageFromTable(); break;
        case 5: openImageFromList(); break;
        case 6: changeRange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: changeBackgroundRange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: changeBackgroundLevel((*reinterpret_cast< real(*)>(_a[1]))); break;
        case 9: updateCenter(); break;
        case 10: updateBeamstop(); break;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void Look::backgroundChecked(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Look::hitChecked(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Look::imgFromListChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
