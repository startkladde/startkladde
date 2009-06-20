/****************************************************************************
** Meta object code from reading C++ file 'sk_combo_box.h'
**
** Created: Thu Jun 18 16:10:52 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_combo_box.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_combo_box.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_combo_box[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      26,   13,   13,   13, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_sk_combo_box[] = {
    "sk_combo_box\0\0focus_out()\0focus_in()\0"
};

const QMetaObject sk_combo_box::staticMetaObject = {
    { &QComboBox::staticMetaObject, qt_meta_stringdata_sk_combo_box,
      qt_meta_data_sk_combo_box, 0 }
};

const QMetaObject *sk_combo_box::metaObject() const
{
    return &staticMetaObject;
}

void *sk_combo_box::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_combo_box))
	return static_cast<void*>(const_cast< sk_combo_box*>(this));
    return QComboBox::qt_metacast(_clname);
}

int sk_combo_box::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QComboBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: focus_out(); break;
        case 1: focus_in(); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void sk_combo_box::focus_out()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void sk_combo_box::focus_in()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
