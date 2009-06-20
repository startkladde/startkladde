/****************************************************************************
** Meta object code from reading C++ file 'splash.h'
**
** Created: Thu Jun 18 16:11:23 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "splash.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'splash.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_splash[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
      18,    7,    7,    7, 0x0a,
      32,    7,    7,    7, 0x0a,
      47,    7,    7,    7, 0x0a,
      61,    7,    7,    7, 0x0a,
      83,   81,    7,    7, 0x0a,
     105,    7,    7,    7, 0x0a,
     117,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_splash[] = {
    "splash\0\0clicked()\0show_splash()\0"
    "show_version()\0hide_splash()\0"
    "set_status(QString)\0,\0set_progress(int,int)\0"
    "try_close()\0close()\0"
};

const QMetaObject splash::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_splash,
      qt_meta_data_splash, 0 }
};

const QMetaObject *splash::metaObject() const
{
    return &staticMetaObject;
}

void *splash::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_splash))
	return static_cast<void*>(const_cast< splash*>(this));
    return QDialog::qt_metacast(_clname);
}

int splash::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: clicked(); break;
        case 1: show_splash(); break;
        case 2: show_version(); break;
        case 3: hide_splash(); break;
        case 4: set_status((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: set_progress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: try_close(); break;
        case 7: close(); break;
        }
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void splash::clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
