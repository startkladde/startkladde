/****************************************************************************
** Meta object code from reading C++ file 'weather_widget.h'
**
** Created: Thu Jun 18 16:11:23 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "weather_widget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weather_widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_weather_widget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      32,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      56,   51,   15,   15, 0x0a,
      75,   15,   15,   15, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_weather_widget[] = {
    "weather_widget\0\0doubleClicked()\0"
    "sizeChanged(QSize)\0line\0inputLine(QString)\0"
    "pluginNotFound()\0"
};

const QMetaObject weather_widget::staticMetaObject = {
    { &QLabel::staticMetaObject, qt_meta_stringdata_weather_widget,
      qt_meta_data_weather_widget, 0 }
};

const QMetaObject *weather_widget::metaObject() const
{
    return &staticMetaObject;
}

void *weather_widget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_weather_widget))
	return static_cast<void*>(const_cast< weather_widget*>(this));
    return QLabel::qt_metacast(_clname);
}

int weather_widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: doubleClicked(); break;
        case 1: sizeChanged((*reinterpret_cast< const QSize(*)>(_a[1]))); break;
        case 2: inputLine((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: pluginNotFound(); break;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void weather_widget::doubleClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void weather_widget::sizeChanged(const QSize & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
