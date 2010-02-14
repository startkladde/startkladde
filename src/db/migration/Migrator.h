#ifndef MIGRATOR_H_
#define MIGRATOR_H_

#include <QString>

class Database;

class Migrator
{
	public:
		static const QString migrationsTableName;

		Migrator (Database &database);
		virtual ~Migrator ();

		void up ();
		void down ();

		QString getVersion ();
//		void addVersion (QString version);
//		void removeVersion (QString version);

	private:
		Database &database;
};

#endif /* MIGRATOR_H_ */
