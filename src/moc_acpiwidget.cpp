/****************************************************************************
** Meta object code from reading C++ file 'acpiwidget.h'
**
** Created: Thu Jun 18 16:10:49 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "acpiwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'acpiwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_acpiwidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_acpiwidget[] = {
    "acpiwidget\0\0slotTimer()\0"
};

const QMetaObject acpiwidget::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_acpiwidget,
      qt_meta_data_acpiwidget, 0 }
};

const QMetaObject *acpiwidget::metaObject() const
{
    return &staticMetaObject;
}

void *acpiwidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_acpiwidget))
	return static_cast<void*>(const_cast< acpiwidget*>(this));
    return QLabel::qt_metacast(_clname);
}

int acpiwidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: slotTimer(); break;
        }
        _id -= 1;
    }
    return _id;
}
