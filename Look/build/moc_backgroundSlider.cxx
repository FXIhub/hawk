/****************************************************************************
** Meta object code from reading C++ file 'backgroundSlider.h'
**
** Created: Tue Apr 15 14:20:55 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../backgroundSlider.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backgroundSlider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_BackgroundSlider[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   37,   17,   17, 0x0a,
      62,   37,   17,   17, 0x0a,
      81,   37,   17,   17, 0x0a,
     100,   37,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_BackgroundSlider[] = {
    "BackgroundSlider\0\0valueChanged(real)\0"
    "value\0minChanged(double)\0maxChanged(double)\0"
    "sliderChanged(int)\0levelBoxChanged(double)\0"
};

const QMetaObject BackgroundSlider::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_BackgroundSlider,
      qt_meta_data_BackgroundSlider, 0 }
};

const QMetaObject *BackgroundSlider::metaObject() const
{
    return &staticMetaObject;
}

void *BackgroundSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BackgroundSlider))
	return static_cast<void*>(const_cast< BackgroundSlider*>(this));
    return QDialog::qt_metacast(_clname);
}

int BackgroundSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: valueChanged((*reinterpret_cast< real(*)>(_a[1]))); break;
        case 1: minChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: maxChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: sliderChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: levelBoxChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void BackgroundSlider::valueChanged(real _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
