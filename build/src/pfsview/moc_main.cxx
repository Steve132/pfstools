/****************************************************************************
** Meta object code from reading C++ file 'main.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/pfsview/main.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PFSViewMainWin[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   36,   36,   36, 0x0a,
      37,   36,   36,   36, 0x0a,
      57,   36,   36,   36, 0x0a,
      82,  107,   36,   36, 0x0a,
     123,   36,   36,   36, 0x0a,
     141,   36,   36,   36, 0x0a,
     157,   36,   36,   36, 0x0a,
     177,   36,   36,   36, 0x0a,
     189,   36,   36,   36, 0x0a,
     201,  226,   36,   36, 0x0a,
     233,  263,   36,   36, 0x0a,
     270,  263,   36,   36, 0x0a,
     294,   36,   36,   36, 0x0a,
     312,   36,   36,   36, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PFSViewMainWin[] = {
    "PFSViewMainWin\0updatePointerValue()\0"
    "\0updateRangeWindow()\0updateChannelSelection()\0"
    "setChannelSelection(int)\0selectedChannel\0"
    "updateZoomValue()\0gotoNextFrame()\0"
    "gotoPreviousFrame()\0saveImage()\0"
    "copyImage()\0setLumMappingMethod(int)\0"
    "method\0setLumMappingMethod(QAction*)\0"
    "action\0setColorCoord(QAction*)\0"
    "showAboutdialog()\0updateViewSize()\0"
};

void PFSViewMainWin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PFSViewMainWin *_t = static_cast<PFSViewMainWin *>(_o);
        switch (_id) {
        case 0: _t->updatePointerValue(); break;
        case 1: _t->updateRangeWindow(); break;
        case 2: _t->updateChannelSelection(); break;
        case 3: _t->setChannelSelection((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updateZoomValue(); break;
        case 5: _t->gotoNextFrame(); break;
        case 6: _t->gotoPreviousFrame(); break;
        case 7: _t->saveImage(); break;
        case 8: _t->copyImage(); break;
        case 9: _t->setLumMappingMethod((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->setLumMappingMethod((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 11: _t->setColorCoord((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 12: _t->showAboutdialog(); break;
        case 13: _t->updateViewSize(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PFSViewMainWin::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PFSViewMainWin::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_PFSViewMainWin,
      qt_meta_data_PFSViewMainWin, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PFSViewMainWin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PFSViewMainWin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PFSViewMainWin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PFSViewMainWin))
        return static_cast<void*>(const_cast< PFSViewMainWin*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int PFSViewMainWin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
