/****************************************************************************
** Meta object code from reading C++ file 'pfsview_widget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/pfsview/pfsview_widget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pfsview_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PFSViewWidgetArea[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_PFSViewWidgetArea[] = {
    "PFSViewWidgetArea\0"
};

void PFSViewWidgetArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PFSViewWidgetArea::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PFSViewWidgetArea::staticMetaObject = {
    { &QScrollArea::staticMetaObject, qt_meta_stringdata_PFSViewWidgetArea,
      qt_meta_data_PFSViewWidgetArea, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PFSViewWidgetArea::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PFSViewWidgetArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PFSViewWidgetArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PFSViewWidgetArea))
        return static_cast<void*>(const_cast< PFSViewWidgetArea*>(this));
    return QScrollArea::qt_metacast(_clname);
}

int PFSViewWidgetArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_PFSViewWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   35,   35,   35, 0x05,

 // slots: signature, parameters, type, tag, flags
      36,   35,   35,   35, 0x0a,
      45,   35,   35,   35, 0x0a,
      55,   35,   35,   35, 0x0a,
      70,  101,   35,   35, 0x0a,
     108,  101,   35,   35, 0x0a,
     137,  101,   35,   35, 0x0a,
     168,  193,   35,   35, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PFSViewWidget[] = {
    "PFSViewWidget\0updatePointerValue()\0\0"
    "zoomIn()\0zoomOut()\0zoomOriginal()\0"
    "setRGBClippingMethod(QAction*)\0action\0"
    "setInfNaNTreatment(QAction*)\0"
    "setNegativeTreatment(QAction*)\0"
    "setLumMappingMethod(int)\0method\0"
};

void PFSViewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PFSViewWidget *_t = static_cast<PFSViewWidget *>(_o);
        switch (_id) {
        case 0: _t->updatePointerValue(); break;
        case 1: _t->zoomIn(); break;
        case 2: _t->zoomOut(); break;
        case 3: _t->zoomOriginal(); break;
        case 4: _t->setRGBClippingMethod((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 5: _t->setInfNaNTreatment((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 6: _t->setNegativeTreatment((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 7: _t->setLumMappingMethod((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PFSViewWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PFSViewWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PFSViewWidget,
      qt_meta_data_PFSViewWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PFSViewWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PFSViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PFSViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PFSViewWidget))
        return static_cast<void*>(const_cast< PFSViewWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int PFSViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void PFSViewWidget::updatePointerValue()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
