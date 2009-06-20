/****************************************************************************
** Meta object code from reading C++ file 'sk_win_stuff_select.h'
**
** Created: Thu Jun 18 16:11:19 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_stuff_select.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_stuff_select.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_selector_helper[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      45,   27,   16,   16, 0x0a,
      90,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_selector_helper[] = {
    "selector_helper\0\0slot_ok()\0item,point,column\0"
    "slot_double_click(QListViewItem*,QPoint,int)\0"
    "reject()\0"
};

const QMetaObject selector_helper::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_selector_helper,
      qt_meta_data_selector_helper, 0 }
};

const QMetaObject *selector_helper::metaObject() const
{
    return &staticMetaObject;
}

void *selector_helper::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_selector_helper))
	return static_cast<void*>(const_cast< selector_helper*>(this));
    return QObject::qt_metacast(_clname);
}

int selector_helper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slot_ok(); break;
        case 1: slot_double_click((*reinterpret_cast< QListViewItem*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: reject(); break;
        }
        _id -= 3;
    }
    return _id;
}
