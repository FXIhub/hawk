/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Wed Apr 23 09:11:36 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_MainWindow[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      38,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      37,   11,   11,   11, 0x08,
      57,   11,   11,   11, 0x08,
      75,   11,   11,   11, 0x08,
      93,   11,   11,   11, 0x08,
     111,   11,   11,   11, 0x08,
     134,   11,   11,   11, 0x08,
     157,   11,   11,   11, 0x08,
     177,   11,   11,   11, 0x08,
     193,   11,   11,   11, 0x08,
     212,   11,   11,   11, 0x08,
     229,   11,   11,   11, 0x08,
     251,   11,   11,   11, 0x08,
     270,   11,   11,   11, 0x08,
     291,   11,   11,   11, 0x08,
     303,   11,   11,   11, 0x08,
     325,   11,   11,   11, 0x08,
     340,   11,   11,   11, 0x08,
     360,   11,   11,   11, 0x08,
     384,   11,   11,   11, 0x08,
     400,   11,   11,   11, 0x08,
     422,   11,   11,   11, 0x08,
     446,   11,   11,   11, 0x08,
     462,   11,   11,   11, 0x08,
     482,   11,   11,   11, 0x08,
     498,   11,   11,   11, 0x08,
     521,   11,   11,   11, 0x08,
     549,   11,   11,   11, 0x08,
     570,   11,   11,   11, 0x08,
     586,   11,   11,   11, 0x08,
     618,   11,   11,   11, 0x08,
     645,   11,   11,   11, 0x08,
     676,   11,   11,   11, 0x08,
     707,   11,   11,   11, 0x08,
     728,   11,   11,   11, 0x08,
     751,   11,   11,   11, 0x08,
     772,   11,   11,   11, 0x08,
     789,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0imgFromListChanged(bool)\0"
    "fileOpenDirectory()\0fileGetComments()\0"
    "fileExportToDir()\0fileExportImage()\0"
    "fileExportFalseColor()\0fileExportBackground()\0"
    "fileSetProperties()\0imageToImages()\0"
    "imageRemoveImage()\0imageSetCenter()\0"
    "imageDefineBeamstop()\0imageCenterToAll()\0"
    "imageBeamstopToAll()\0imageCrop()\0"
    "viewAutocorrelation()\0viewLogScale()\0"
    "viewPredefineSize()\0viewUsePredefinedSize()\0"
    "viewDistances()\0colorscaleGrayscale()\0"
    "colorscaleTraditional()\0colorscaleHot()\0"
    "colorscaleRainbow()\0colorscaleJet()\0"
    "categorizeBackground()\0"
    "categorizeClearBackground()\0"
    "categorizeClearHit()\0categorizeHit()\0"
    "categorizeCalculateBackground()\0"
    "categorizeShowBackground()\0"
    "categorizeSubtractBackground()\0"
    "categorizeSetBackgroundLevel()\0"
    "maskBeamstopToMask()\0maskSaturationToMask()\0"
    "maskVertLineToMask()\0maskImportMask()\0"
    "maskShowMask()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

const QMetaObject *MainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
	return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: imgFromListChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: fileOpenDirectory(); break;
        case 2: fileGetComments(); break;
        case 3: fileExportToDir(); break;
        case 4: fileExportImage(); break;
        case 5: fileExportFalseColor(); break;
        case 6: fileExportBackground(); break;
        case 7: fileSetProperties(); break;
        case 8: imageToImages(); break;
        case 9: imageRemoveImage(); break;
        case 10: imageSetCenter(); break;
        case 11: imageDefineBeamstop(); break;
        case 12: imageCenterToAll(); break;
        case 13: imageBeamstopToAll(); break;
        case 14: imageCrop(); break;
        case 15: viewAutocorrelation(); break;
        case 16: viewLogScale(); break;
        case 17: viewPredefineSize(); break;
        case 18: viewUsePredefinedSize(); break;
        case 19: viewDistances(); break;
        case 20: colorscaleGrayscale(); break;
        case 21: colorscaleTraditional(); break;
        case 22: colorscaleHot(); break;
        case 23: colorscaleRainbow(); break;
        case 24: colorscaleJet(); break;
        case 25: categorizeBackground(); break;
        case 26: categorizeClearBackground(); break;
        case 27: categorizeClearHit(); break;
        case 28: categorizeHit(); break;
        case 29: categorizeCalculateBackground(); break;
        case 30: categorizeShowBackground(); break;
        case 31: categorizeSubtractBackground(); break;
        case 32: categorizeSetBackgroundLevel(); break;
        case 33: maskBeamstopToMask(); break;
        case 34: maskSaturationToMask(); break;
        case 35: maskVertLineToMask(); break;
        case 36: maskImportMask(); break;
        case 37: maskShowMask(); break;
        }
        _id -= 38;
    }
    return _id;
}
