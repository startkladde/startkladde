#ifndef MIGRATION_20100216180053_FULL_PLANE_CATEGORY_H_
#define MIGRATION_20100216180053_FULL_PLANE_CATEGORY_H_

#include "src/db/migration/Migration.h"

class QString;

/**
 * Changes the plane.category column from a character to a string
 */
class Migration_20100216180053_full_plane_category: public Migration
{
	public:
		Migration_20100216180053_full_plane_category (Database &database);
		virtual ~Migration_20100216180053_full_plane_category ();

		virtual void up ();
		virtual void down ();

	protected:
		void updateValue (const QString &oldValue, const QString &newValue, Direction direction);
		void updateValues (Direction direction);
};

#endif

