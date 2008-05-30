/****************************************************************************
** Meta object code from reading C++ file 'propertiesDialog.h'
**
** Created: Mon Apr 14 20:38:08 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../propertiesDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'propertiesDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_PropertiesDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PropertiesDialog[] = {
    "PropertiesDialog\0\0close()\0"
};

const QMetaObject PropertiesDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PropertiesDialog,
      qt_meta_data_PropertiesDialog, 0 }
};

const QMetaObject *PropertiesDialog::metaObject() const
{
    return &staticMetaObject;
}

void *PropertiesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PropertiesDialog))
	return static_cast<void*>(const_cast< PropertiesDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PropertiesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: close(); break;
        }
        _id -= 1;
    }
    return _id;
}
