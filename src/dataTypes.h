#ifndef _dataTypes_h
#define _dataTypes_h

#include <cstdio>
#include <QString>
#include <QList>
#include <string>

#include <mysql.h>

#include "src/logging/messages.h"

enum FlightType { ftNone, ftNormal, ftTraining2, ftTraining1, ftTow, ftGuestPrivate, ftGuestExternal };
enum FlightMode { fmNone, fmLocal, fmComing, fmLeaving };

enum flight_manipulation { fm_start, fm_land, fm_land_schlepp, fm_edit, fm_delete, fm_zwischenlandung, fm_wiederholen };

enum lengthSpecification { lsShort, lsTable, lsLong, lsWithShortcut, lsPrintout, lsCsv, lsPilotLog };
enum casus { cas_nominativ, cas_genitiv, cas_dativ, cas_akkusativ };

enum db_event_type { det_none, det_add, det_delete, det_change, det_refresh };
enum db_event_table { db_kein, db_alle, db_person, db_flug, db_flugzeug };

enum EntityType { st_none, st_plane, st_person, st_startart };

QList<FlightType> listFlightTypes (bool include_invalid);
QString flightTypeText (FlightType flightType, lengthSpecification lenspec);
bool begleiter_erlaubt (FlightType flugtyp);

QList<FlightMode> listFlightModes (bool);
QList<FlightMode> listTowFlightModes (bool);
QString flightModeText (FlightMode, lengthSpecification);

bool ist_schulung (FlightType t);
bool lands_here (FlightMode m);
bool starts_here (FlightMode m);


//QString flugstatus_string (flug_status, lengthSpecification lenspec);

QString std2q (std::string);
std::string q2std (QString);

db_event_table TableFromEntityType (EntityType);

#endif

