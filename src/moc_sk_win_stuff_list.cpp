/****************************************************************************
** Meta object code from reading C++ file 'sk_win_stuff_list.h'
**
** Created: Thu Jun 18 16:11:16 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_stuff_list.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_stuff_list.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_win_stuff_list[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      23,   19,   18,   18, 0x08,
      67,   18,   18,   18, 0x08,
      87,   18,   18,   18, 0x08,
      98,   18,   18,   18, 0x08,
     114,   18,   18,   18, 0x08,
     129,   18,   18,   18, 0x08,
     139,   18,   18,   18, 0x08,
     156,   18,   18,   18, 0x08,
     173,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_win_stuff_list[] = {
    "sk_win_stuff_list\0\0,,,\0"
    "slot_table_double_click(int,int,int,QPoint)\0"
    "slot_table_key(int)\0slot_neu()\0"
    "slot_loeschen()\0slot_refresh()\0slot_ok()\0"
    "slot_abbrechen()\0slot_editieren()\0"
    "slot_db_update(db_event*)\0"
};

const QMetaObject sk_win_stuff_list::staticMetaObject = {
    { &sk_dialog::staticMetaObject, qt_meta_stringdata_sk_win_stuff_list,
      qt_meta_data_sk_win_stuff_list, 0 }
};

const QMetaObject *sk_win_stuff_list::metaObject() const
{
    return &staticMetaObject;
}

void *sk_win_stuff_list::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_win_stuff_list))
	return static_cast<void*>(const_cast< sk_win_stuff_list*>(this));
    return sk_dialog::qt_metacast(_clname);
}

int sk_win_stuff_list::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = sk_dialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slot_table_double_click((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const QPoint(*)>(_a[4]))); break;
        case 1: slot_table_key((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: slot_neu(); break;
        case 3: slot_loeschen(); break;
        case 4: slot_refresh(); break;
        case 5: slot_ok(); break;
        case 6: slot_abbrechen(); break;
        case 7: slot_editieren(); break;
        case 8: slot_db_update((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        }
        _id -= 9;
    }
    return _id;
}
