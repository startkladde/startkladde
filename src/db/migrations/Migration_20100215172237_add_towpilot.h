#ifndef MIGRATION_20100215172237_ADD_TOWPILOT_H_
#define MIGRATION_20100215172237_ADD_TOWPILOT_H_

#include "src/db/migration/Migration.h"

/**
 * Adds the towpilot columns to the flight table, if they don't exist yet.
 *
 * The towpilot columns were optional before, so this migration creates a
 * defined status.
 */
class Migration_20100215172237_add_towpilot: public Migration
{
	public:
		Migration_20100215172237_add_towpilot (Database &database);
		virtual ~Migration_20100215172237_add_towpilot ();

		virtual void up ();
		virtual void down ();
};

#endif

