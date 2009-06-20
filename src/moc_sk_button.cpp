/****************************************************************************
** Meta object code from reading C++ file 'sk_button.h'
**
** Created: Thu Jun 18 16:10:51 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_button.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_button.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_button[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      13,   11,   10,   10, 0x05,
      34,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      49,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_sk_button[] = {
    "sk_button\0\0,\0clicked(db_id,db_id)\0"
    "clicked(db_id)\0slot_clicked()\0"
};

const QMetaObject sk_button::staticMetaObject = {
    { &QPushButton::staticMetaObject, qt_meta_stringdata_sk_button,
      qt_meta_data_sk_button, 0 }
};

const QMetaObject *sk_button::metaObject() const
{
    return &staticMetaObject;
}

void *sk_button::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_button))
	return static_cast<void*>(const_cast< sk_button*>(this));
    return QPushButton::qt_metacast(_clname);
}

int sk_button::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: clicked((*reinterpret_cast< db_id(*)>(_a[1])),(*reinterpret_cast< db_id(*)>(_a[2]))); break;
        case 1: clicked((*reinterpret_cast< db_id(*)>(_a[1]))); break;
        case 2: slot_clicked(); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void sk_button::clicked(db_id _t1, db_id _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void sk_button::clicked(db_id _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
