#include "Migration_20100216124307_rename_columns.h"

#include <QString>

#include "src/db/Database.h"

Migration_20100216124307_rename_columns::Migration_20100216124307_rename_columns (Database &database):
	Migration (database)
{
}

Migration_20100216124307_rename_columns::~Migration_20100216124307_rename_columns ()
{
}

void Migration_20100216124307_rename_columns::up ()
{
	renameColumns (dirUp);
}

void Migration_20100216124307_rename_columns::down ()
{
	renameColumns (dirDown);
}

void Migration_20100216124307_rename_columns::renameColumn (const QString &table, const QString &oldName, const QString &newName, const QString &type, Migration::Direction direction)
{
	switch (direction)
	{
		case dirUp:   database.renameColumn (table, oldName, newName, type); break;
		case dirDown: database.renameColumn (table, newName, oldName, type); break;
	}
}

void Migration_20100216124307_rename_columns::renameColumns (Direction direction)
{
	// *** People
	renameColumn ("people", "nachname"  , "last_name" , Database::dataTypeString, direction);
	renameColumn ("people", "vorname"   , "first_name", Database::dataTypeString, direction);
	renameColumn ("people", "verein"    , "club"      , Database::dataTypeString, direction);
	renameColumn ("people", "spitzname" , "nickname"  , Database::dataTypeString, direction);
	renameColumn ("people", "vereins_id", "club_id"   , Database::dataTypeString, direction);
	renameColumn ("people", "bemerkung" , "comments"  , Database::dataTypeString, direction);

	// *** Planes
	renameColumn ("planes", "kennzeichen"           , "registration"              ,  Database::dataTypeString   , direction);
	renameColumn ("planes", "verein"                , "club"                      ,  Database::dataTypeString   , direction);
	renameColumn ("planes", "sitze"                 , "num_seats"                 ,  Database::dataTypeInteger  , direction);
	renameColumn ("planes", "typ"                   , "type"                      ,  Database::dataTypeString   , direction);
	renameColumn ("planes", "gattung"               , "category"                  ,  Database::dataTypeCharacter, direction);
	renameColumn ("planes", "wettbewerbskennzeichen", "competition_callsign"      ,  Database::dataTypeString   , direction);
	renameColumn ("planes", "bemerkung"             , "comments"                  ,  Database::dataTypeString   , direction);

	// *** Flights
	renameColumn ("flights", "flugzeug"             , "plane_id"                  ,  Database::dataTypeId       , direction);
	renameColumn ("flights", "pilot"                , "pilot_id"                  ,  Database::dataTypeId       , direction);
	renameColumn ("flights", "begleiter"            , "copilot_id"                ,  Database::dataTypeId       , direction);
	// Flight settings
	renameColumn ("flights", "typ"                  , "type"                      ,  Database::dataTypeInteger  , direction);
	renameColumn ("flights", "modus"                , "mode"                      ,  Database::dataTypeCharacter, direction);
	// Takeoff and landing
	renameColumn ("flights", "status"               , "status"                    ,  Database::dataTypeInteger  , direction);
	renameColumn ("flights", "startart"             , "launch_method_id"          ,  Database::dataTypeId       , direction);
	renameColumn ("flights", "startort"             , "departure_location"        ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "zielort"              , "landing_location"          ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "anzahl_landungen"     , "num_landings"              ,  Database::dataTypeInteger  , direction);
	renameColumn ("flights", "startzeit"            , "departure_time"            ,  Database::dataTypeDatetime , direction);
	renameColumn ("flights", "landezeit"            , "landing_time"              ,  Database::dataTypeDatetime , direction);
	// Towflight
	renameColumn ("flights", "towplane"             , "towplane_id"               ,  Database::dataTypeId       , direction);
	renameColumn ("flights", "modus_sfz"            , "towflight_mode"            ,  Database::dataTypeCharacter, direction);
	renameColumn ("flights", "zielort_sfz"          , "towflight_landing_location",  Database::dataTypeString   , direction);
	renameColumn ("flights", "land_schlepp"         , "towflight_landing_time"    ,  Database::dataTypeDatetime , direction);
	renameColumn ("flights", "towpilot"             , "towpilot_id"               ,  Database::dataTypeId       , direction);
	// Incomplete names
	renameColumn ("flights", "pvn"                  , "pilot_first_name"          ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "pnn"                  , "pilot_last_name"           ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "bvn"                  , "copilot_first_name"        ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "bnn"                  , "copilot_last_name"         ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "tpvn"                 , "towpilot_first_name"       ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "tpnn"                 , "towpilot_last_name"        ,  Database::dataTypeString   , direction);
	// Comments
	renameColumn ("flights", "bemerkung"            , "comments"         ,  Database::dataTypeString   , direction);
	renameColumn ("flights", "abrechnungshinweis"   , "accounting_notes" ,  Database::dataTypeString   , direction);

	// *** Users
	renameColumn ("users", "username"           , "username"           ,  Database::dataTypeString , direction); // TODO not null
	renameColumn ("users", "password"           , "password"           ,  Database::dataTypeString , direction); // TODO long enough?
	renameColumn ("users", "perm_club_admin"    , "perm_club_admin"    ,  Database::dataTypeBoolean, direction);
	renameColumn ("users", "perm_read_flight_db", "perm_read_flight_db",  Database::dataTypeBoolean, direction);
	renameColumn ("users", "club"               , "club"               ,  Database::dataTypeString , direction);
	renameColumn ("users", "person"             , "person_id"          ,  Database::dataTypeId     , direction);
}
