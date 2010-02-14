#ifndef MIGRATION_H_
#define MIGRATION_H_

class Database;

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
