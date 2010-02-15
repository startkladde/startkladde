#ifndef MIGRATION_H_
#define MIGRATION_H_

class Database;

/**
 * Migration are supposed to be called only through Migrator, which keeps track
 * of which migrations have already been applied
 */
class Migration
{
	public:
		Migration (Database &database);
		virtual ~Migration ();

		virtual void up ()=0;
		virtual void down ()=0;

	protected:
		Database &database;
};

#endif /* MIGRATION_H_ */
