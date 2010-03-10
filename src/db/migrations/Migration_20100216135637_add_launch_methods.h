#ifndef MIGRATION_20100216135637_ADD_LAUNCH_METHODS_H_
#define MIGRATION_20100216135637_ADD_LAUNCH_METHODS_H_

#include "src/db/migration/Migration.h"

/**
 * Adds the launch methods table (and imports the launch methods from the
 * configuration file)
 */
class Migration_20100216135637_add_launch_methods: public Migration
{
	public:
		Migration_20100216135637_add_launch_methods (Interface &interface);
		virtual ~Migration_20100216135637_add_launch_methods ();

		virtual void up ();
		virtual void down ();
};

#endif

