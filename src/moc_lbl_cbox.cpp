/****************************************************************************
** Meta object code from reading C++ file 'lbl_cbox.h'
**
** Created: Thu Jun 18 16:10:50 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "lbl_cbox.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'lbl_cbox.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_lbl_cbox[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      17,   10,    9,    9, 0x05,
      36,   10,    9,    9, 0x05,
      57,    9,    9,    9, 0x05,
      71,   69,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      89,    9,    9,    9, 0x0a,
      97,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_lbl_cbox[] = {
    "lbl_cbox\0\0string\0activated(QString)\0"
    "highlighted(QString)\0focus_out()\0,\0"
    "progress(int,int)\0clear()\0some_focus_out()\0"
};

const QMetaObject lbl_cbox::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_lbl_cbox,
      qt_meta_data_lbl_cbox, 0 }
};

const QMetaObject *lbl_cbox::metaObject() const
{
    return &staticMetaObject;
}

void *lbl_cbox::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_lbl_cbox))
	return static_cast<void*>(const_cast< lbl_cbox*>(this));
    return QFrame::qt_metacast(_clname);
}

int lbl_cbox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: highlighted((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: focus_out(); break;
        case 3: progress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: clear(); break;
        case 5: some_focus_out(); break;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void lbl_cbox::activated(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void lbl_cbox::highlighted(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void lbl_cbox::focus_out()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void lbl_cbox::progress(int _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
