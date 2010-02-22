#ifndef MIGRATION_20100215211913_DROP_OLD_COLUMNS_H_
#define MIGRATION_20100215211913_DROP_OLD_COLUMNS_H_

#include "src/db/migration/Migration.h"

/**
 * Removes some columns which were created in the legacy database, but never
 * used.
 *
 * This change is not undone by the down migration; see 20100214140000.
 */
class Migration_20100215211913_drop_old_columns: public Migration
{
	public:
		Migration_20100215211913_drop_old_columns (DatabaseInterface &databaseInterface);
		virtual ~Migration_20100215211913_drop_old_columns ();

		virtual void up ();
		virtual void down ();
};

#endif

