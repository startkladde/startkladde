#ifndef _dataTypes_h
#define _dataTypes_h

#include <cstdio>
#include <QString>

#include <string>

#include <mysql.h>

#include "src/logging/messages.h"

// TODO: change uebung to schulung1, schulung to schulung2
enum flug_typ { ft_kein, ft_normal, ft_schul_2, ft_schul_1, ft_schlepp, ft_gast_privat, ft_gast_extern };
enum flug_modus { fmod_kein, fmod_lokal, fmod_kommt, fmod_geht };

enum flight_manipulation { fm_start, fm_land, fm_land_schlepp, fm_edit, fm_delete, fm_zwischenlandung, fm_wiederholen };

enum lengthSpecification { lsShort, lsTable, lsLong, lsWithShortcut, lsPrintout, lsCsv, lsPilotLog };
enum casus { cas_nominativ, cas_genitiv, cas_dativ, cas_akkusativ };

enum db_event_type { det_none, det_add, det_delete, det_change, det_refresh };
enum db_event_table { db_kein, db_alle, db_person, db_flug, db_flugzeug };

enum EntityType { st_none, st_plane, st_person, st_startart };

int list_flugtyp (flug_typ **t, bool include_invalid);
QString flugtyp_string (flug_typ flugtyp, lengthSpecification lenspec);
bool begleiter_erlaubt (flug_typ flugtyp);

int list_modus (flug_modus **, bool);
int list_sfz_modus (flug_modus **, bool);
QString modus_string (flug_modus, lengthSpecification);

bool ist_schulung (flug_typ t);
bool lands_here (flug_modus m);
bool starts_here (flug_modus m);


//QString flugstatus_string (flug_status, lengthSpecification lenspec);

QString std2q (std::string);
std::string q2std (QString);

db_event_table TableFromEntityType (EntityType);

#endif

