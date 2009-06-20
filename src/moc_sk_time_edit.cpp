/****************************************************************************
** Meta object code from reading C++ file 'sk_time_edit.h'
**
** Created: Thu Jun 18 16:11:04 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_time_edit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_time_edit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_time_edit[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      24,   13,   13,   13, 0x05,
      39,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      54,   13,   13,   13, 0x0a,
      73,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_sk_time_edit[] = {
    "sk_time_edit\0\0clicked()\0time_changed()\0"
    "cbox_clicked()\0set_current_time()\0"
    "update_cbox()\0"
};

const QMetaObject sk_time_edit::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_sk_time_edit,
      qt_meta_data_sk_time_edit, 0 }
};

const QMetaObject *sk_time_edit::metaObject() const
{
    return &staticMetaObject;
}

void *sk_time_edit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_time_edit))
	return static_cast<void*>(const_cast< sk_time_edit*>(this));
    return QFrame::qt_metacast(_clname);
}

int sk_time_edit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: clicked(); break;
        case 1: time_changed(); break;
        case 2: cbox_clicked(); break;
        case 3: set_current_time(); break;
        case 4: update_cbox(); break;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void sk_time_edit::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void sk_time_edit::time_changed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void sk_time_edit::cbox_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
