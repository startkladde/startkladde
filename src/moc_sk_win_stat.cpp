/****************************************************************************
** Meta object code from reading C++ file 'sk_win_stat.h'
**
** Created: Thu Jun 18 16:11:10 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_stat.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_stat.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_win_stat[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_sk_win_stat[] = {
    "sk_win_stat\0"
};

const QMetaObject sk_win_stat::staticMetaObject = {
    { &sk_dialog::staticMetaObject, qt_meta_stringdata_sk_win_stat,
      qt_meta_data_sk_win_stat, 0 }
};

const QMetaObject *sk_win_stat::metaObject() const
{
    return &staticMetaObject;
}

void *sk_win_stat::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_win_stat))
	return static_cast<void*>(const_cast< sk_win_stat*>(this));
    return sk_dialog::qt_metacast(_clname);
}

int sk_win_stat::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = sk_dialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
