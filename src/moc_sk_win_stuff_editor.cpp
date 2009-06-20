/****************************************************************************
** Meta object code from reading C++ file 'sk_win_stuff_editor.h'
**
** Created: Thu Jun 18 16:11:13 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_stuff_editor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_stuff_editor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_win_stuff_editor[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      33,   20,   20,   20, 0x08,
      53,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_win_stuff_editor[] = {
    "sk_win_stuff_editor\0\0slot_save()\0"
    "slot_registration()\0slot_db_update(db_event*)\0"
};

const QMetaObject sk_win_stuff_editor::staticMetaObject = {
    { &sk_dialog::staticMetaObject, qt_meta_stringdata_sk_win_stuff_editor,
      qt_meta_data_sk_win_stuff_editor, 0 }
};

const QMetaObject *sk_win_stuff_editor::metaObject() const
{
    return &staticMetaObject;
}

void *sk_win_stuff_editor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_win_stuff_editor))
	return static_cast<void*>(const_cast< sk_win_stuff_editor*>(this));
    return sk_dialog::qt_metacast(_clname);
}

int sk_win_stuff_editor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = sk_dialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slot_save(); break;
        case 1: slot_registration(); break;
        case 2: slot_db_update((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        }
        _id -= 3;
    }
    return _id;
}
