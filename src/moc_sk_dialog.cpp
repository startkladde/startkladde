/****************************************************************************
** Meta object code from reading C++ file 'sk_dialog.h'
**
** Created: Thu Jun 18 16:10:54 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_dialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      17,   11,   10,   10, 0x05,
      38,   11,   10,   10, 0x05,
      59,   10,   10,   10, 0x05,
      77,   75,   10,   10, 0x05,
      95,   10,   10,   10, 0x05,
     118,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     139,   11,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_dialog[] = {
    "sk_dialog\0\0event\0db_change(db_event*)\0"
    "db_update(db_event*)\0status(QString)\0"
    ",\0progress(int,int)\0long_operation_start()\0"
    "long_operation_end()\0slot_db_update(db_event*)\0"
};

const QMetaObject sk_dialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_sk_dialog,
      qt_meta_data_sk_dialog, 0 }
};

const QMetaObject *sk_dialog::metaObject() const
{
    return &staticMetaObject;
}

void *sk_dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_dialog))
	return static_cast<void*>(const_cast< sk_dialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int sk_dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: db_change((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        case 1: db_update((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        case 2: status((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: progress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: long_operation_start(); break;
        case 5: long_operation_end(); break;
        case 6: slot_db_update((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void sk_dialog::db_change(db_event * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void sk_dialog::db_update(db_event * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void sk_dialog::status(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void sk_dialog::progress(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void sk_dialog::long_operation_start()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void sk_dialog::long_operation_end()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}
