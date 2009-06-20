/****************************************************************************
** Meta object code from reading C++ file 'sk_win_date.h'
**
** Created: Thu Jun 18 16:11:04 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_date.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_date.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_win_date[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      34,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      63,   12,   12,   12, 0x0a,
      75,   12,   12,   12, 0x0a,
      95,   12,   91,   12, 0x0a,
     102,   12,   12,   12, 0x08,
     112,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_sk_win_date[] = {
    "sk_win_date\0\0date_accepted(QDate)\0"
    "datetime_accepted(QDateTime)\0edit_date()\0"
    "edit_datetime()\0int\0exec()\0slot_ok()\0"
    "slot_cancel()\0"
};

const QMetaObject sk_win_date::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_sk_win_date,
      qt_meta_data_sk_win_date, 0 }
};

const QMetaObject *sk_win_date::metaObject() const
{
    return &staticMetaObject;
}

void *sk_win_date::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_win_date))
	return static_cast<void*>(const_cast< sk_win_date*>(this));
    return QDialog::qt_metacast(_clname);
}

int sk_win_date::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: date_accepted((*reinterpret_cast< QDate(*)>(_a[1]))); break;
        case 1: datetime_accepted((*reinterpret_cast< QDateTime(*)>(_a[1]))); break;
        case 2: edit_date(); break;
        case 3: edit_datetime(); break;
        case 4: { int _r = exec();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: slot_ok(); break;
        case 6: slot_cancel(); break;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void sk_win_date::date_accepted(QDate _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void sk_win_date::datetime_accepted(QDateTime _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
