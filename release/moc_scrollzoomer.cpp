/****************************************************************************
** Meta object code from reading C++ file 'scrollzoomer.h'
**
** Created: Sat Oct 29 13:30:13 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../scrollzoomer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scrollzoomer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ScrollZoomer[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   14,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ScrollZoomer[] = {
    "ScrollZoomer\0\0o,min,max\0"
    "scrollBarMoved(Qt::Orientation,double,double)\0"
};

const QMetaObject ScrollZoomer::staticMetaObject = {
    { &QwtPlotZoomer::staticMetaObject, qt_meta_stringdata_ScrollZoomer,
      qt_meta_data_ScrollZoomer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ScrollZoomer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ScrollZoomer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ScrollZoomer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ScrollZoomer))
        return static_cast<void*>(const_cast< ScrollZoomer*>(this));
    return QwtPlotZoomer::qt_metacast(_clname);
}

int ScrollZoomer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtPlotZoomer::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: scrollBarMoved((*reinterpret_cast< Qt::Orientation(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
