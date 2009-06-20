/****************************************************************************
** Meta object code from reading C++ file 'sk_win_flight.h'
**
** Created: Thu Jun 18 16:11:05 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sk_win_flight.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sk_win_flight.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_sk_win_flight[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      37,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      36,   14,   14,   14, 0x08,
      46,   14,   14,   14, 0x08,
      59,   14,   14,   14, 0x08,
      73,   14,   14,   14, 0x08,
      93,   14,   14,   14, 0x08,
     120,  116,   14,   14, 0x08,
     138,   14,   14,   14, 0x08,
     154,   14,   14,   14, 0x08,
     170,   14,   14,   14, 0x08,
     190,   14,   14,   14, 0x08,
     210,   14,   14,   14, 0x08,
     229,   14,   14,   14, 0x08,
     248,  116,   14,   14, 0x08,
     264,  116,   14,   14, 0x08,
     283,   14,   14,   14, 0x08,
     307,   14,   14,   14, 0x08,
     334,  116,   14,   14, 0x08,
     354,   14,   14,   14, 0x08,
     371,   14,   14,   14, 0x08,
     388,   14,   14,   14, 0x08,
     404,   14,   14,   14, 0x08,
     421,   14,   14,   14, 0x08,
     441,   14,   14,   14, 0x08,
     462,   14,   14,   14, 0x08,
     478,   14,   14,   14, 0x08,
     493,   14,   14,   14, 0x08,
     512,   14,   14,   14, 0x08,
     529,   14,   14,   14, 0x08,
     542,   14,   14,   14, 0x08,
     559,   14,   14,   14, 0x08,
     607,  585,   14,   14, 0x08,
     653,  638,   14,   14, 0x28,
     684,  679,   14,   14, 0x28,
     705,   14,   14,   14, 0x28,
     722,   14,   14,   14, 0x08,
     736,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_sk_win_flight[] = {
    "sk_win_flight\0\0dialog_finished(int)\0"
    "slot_ok()\0slot_later()\0slot_cancel()\0"
    "slot_registration()\0slot_registration_in()\0"
    "ind\0slot_flugtyp(int)\0slot_pilot_vn()\0"
    "slot_pilot_nn()\0slot_begleiter_vn()\0"
    "slot_begleiter_nn()\0slot_towpilot_vn()\0"
    "slot_towpilot_nn()\0slot_modus(int)\0"
    "slot_startart(int)\0slot_registration_sfz()\0"
    "slot_registration_sfz_in()\0"
    "slot_modus_sfz(int)\0slot_gestartet()\0"
    "slot_startzeit()\0slot_gelandet()\0"
    "slot_landezeit()\0slot_sfz_gelandet()\0"
    "slot_landezeit_sfz()\0slot_startort()\0"
    "slot_zielort()\0slot_zielort_sfz()\0"
    "slot_landungen()\0slot_datum()\0"
    "slot_bemerkung()\0slot_abrechnungshinweis()\0"
    "init,read_only,repeat\0"
    "setup_controls(bool,bool,bool)\0"
    "init,read_only\0setup_controls(bool,bool)\0"
    "init\0setup_controls(bool)\0setup_controls()\0"
    "accept_date()\0slot_db_update(db_event*)\0"
};

const QMetaObject sk_win_flight::staticMetaObject = {
    { &sk_dialog::staticMetaObject, qt_meta_stringdata_sk_win_flight,
      qt_meta_data_sk_win_flight, 0 }
};

const QMetaObject *sk_win_flight::metaObject() const
{
    return &staticMetaObject;
}

void *sk_win_flight::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sk_win_flight))
	return static_cast<void*>(const_cast< sk_win_flight*>(this));
    return sk_dialog::qt_metacast(_clname);
}

int sk_win_flight::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = sk_dialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: dialog_finished((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: slot_ok(); break;
        case 2: slot_later(); break;
        case 3: slot_cancel(); break;
        case 4: slot_registration(); break;
        case 5: slot_registration_in(); break;
        case 6: slot_flugtyp((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: slot_pilot_vn(); break;
        case 8: slot_pilot_nn(); break;
        case 9: slot_begleiter_vn(); break;
        case 10: slot_begleiter_nn(); break;
        case 11: slot_towpilot_vn(); break;
        case 12: slot_towpilot_nn(); break;
        case 13: slot_modus((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: slot_startart((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: slot_registration_sfz(); break;
        case 16: slot_registration_sfz_in(); break;
        case 17: slot_modus_sfz((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: slot_gestartet(); break;
        case 19: slot_startzeit(); break;
        case 20: slot_gelandet(); break;
        case 21: slot_landezeit(); break;
        case 22: slot_sfz_gelandet(); break;
        case 23: slot_landezeit_sfz(); break;
        case 24: slot_startort(); break;
        case 25: slot_zielort(); break;
        case 26: slot_zielort_sfz(); break;
        case 27: slot_landungen(); break;
        case 28: slot_datum(); break;
        case 29: slot_bemerkung(); break;
        case 30: slot_abrechnungshinweis(); break;
        case 31: setup_controls((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 32: setup_controls((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 33: setup_controls((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 34: setup_controls(); break;
        case 35: accept_date(); break;
        case 36: slot_db_update((*reinterpret_cast< db_event*(*)>(_a[1]))); break;
        }
        _id -= 37;
    }
    return _id;
}

// SIGNAL 0
void sk_win_flight::dialog_finished(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
