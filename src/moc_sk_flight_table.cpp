/****************************************************************************
** Meta object code from reading C++ file 'sk_flight_table.h'
**
** Created: Thu Jun 18 16:10:55 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_flight_table.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_flight_table.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_flight_table[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x05,
      44,   16,   16,   16, 0x05,
      73,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
     109,   16,   16,   16, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_sk_flight_table[] = {
    "sk_flight_table\0\0signal_button_start(db_id)\0"
    "signal_button_landung(db_id)\0"
    "signal_button_schlepplandung(db_id)\0"
    "columnClicked(int)\0"
};

const QMetaObject sk_flight_table::staticMetaObject = {
    { &sk_table::staticMetaObject, qt_meta_stringdata_sk_flight_table,
      qt_meta_data_sk_flight_table, 0 }
};

const QMetaObject *sk_flight_table::metaObject() const
{
    return &staticMetaObject;
}

void *sk_flight_table::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_flight_table))
	return static_cast<void*>(const_cast< sk_flight_table*>(this));
    return sk_table::qt_metacast(_clname);
}

int sk_flight_table::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = sk_table::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: signal_button_start((*reinterpret_cast< db_id(*)>(_a[1]))); break;
        case 1: signal_button_landung((*reinterpret_cast< db_id(*)>(_a[1]))); break;
        case 2: signal_button_schlepplandung((*reinterpret_cast< db_id(*)>(_a[1]))); break;
        case 3: columnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void sk_flight_table::signal_button_start(db_id _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void sk_flight_table::signal_button_landung(db_id _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void sk_flight_table::signal_button_schlepplandung(db_id _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
