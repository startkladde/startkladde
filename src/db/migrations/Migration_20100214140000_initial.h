#ifndef MIGRATION_20100214_140000_H_
#define MIGRATION_20100214_140000_H_

#include "src/db/migration/Migration.h"

/**
 * Creates the database as it was created by pre-migration versions.
 *
 * There are some differences to the original schema. All of these
 * differences will be updated in a later transaction, but it doesn't
 * hurt to already create them like this:
 *   - The towpilot columns will be created (they were optional before)
 *   - The storage engine will be InnoDB (was MyISAM before)
 *   - The data types may be different
 *   - The charset will be UTF8 (was latin1 before; fortunately, the
 *     conversion is done automatically)
 *   - The order of the columns may be different
 *   - The columns flight.editierbar, flight.vereinand person.bwlv (which
 *     did not contain any useful data) will not be created
 */
class Migration_20100214140000_initial: public Migration
{
	public:
		Migration_20100214140000_initial (Database &database);
		virtual ~Migration_20100214140000_initial ();

		virtual void up ();
		virtual void down ();
};

#endif /* MIGRATION_20100214_140000_H_ */
