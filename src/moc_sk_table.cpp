/****************************************************************************
** Meta object code from reading C++ file 'sk_table.h'
**
** Created: Thu Jun 18 16:10:59 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_table.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_table.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_table[] = {

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
      19,    9,    9,    9, 0x09,
      38,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_table[] = {
    "sk_table\0\0key(int)\0columnClicked(int)\0"
    "removeRow(int)\0"
};

const QMetaObject sk_table::staticMetaObject = {
    { &QTable::staticMetaObject, qt_meta_stringdata_sk_table,
      qt_meta_data_sk_table, 0 }
};

const QMetaObject *sk_table::metaObject() const
{
    return &staticMetaObject;
}

void *sk_table::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_table))
	return static_cast<void*>(const_cast< sk_table*>(this));
    return QTable::qt_metacast(_clname);
}

int sk_table::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTable::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: key((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: columnClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: removeRow((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void sk_table::key(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
