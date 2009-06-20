/****************************************************************************
** Meta object code from reading C++ file 'sk_label.h'
**
** Created: Thu Jun 18 16:10:57 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_label.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_label.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_label[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      27,   20,    9,    9, 0x0a,
      54,   43,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_label[] = {
    "sk_label\0\0clicked()\0_error\0set_error(bool)\0"
    "_invisible\0set_invisible(bool)\0"
};

const QMetaObject sk_label::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_sk_label,
      qt_meta_data_sk_label, 0 }
};

const QMetaObject *sk_label::metaObject() const
{
    return &staticMetaObject;
}

void *sk_label::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_label))
	return static_cast<void*>(const_cast< sk_label*>(this));
    return QLabel::qt_metacast(_clname);
}

int sk_label::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: clicked(); break;
        case 1: set_error((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: set_invisible((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void sk_label::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
