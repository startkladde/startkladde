#ifndef _SchemaDumper_h
#define _SchemaDumper_h

#include <QString>

class Database;

class SchemaDumper
{
	public:
		SchemaDumper (Database &database);
		virtual ~SchemaDumper ();

		QString dumpSchema ();
		void dumpSchemaToFile (const QString &filename);

	protected:
		Database &database;

		void dumpTables   (QStringList &output);
		void dumpTable    (QStringList &output, const QString &name);
		void dumpColumns  (QStringList &output, const QString &table);
		void dumpColumn   (QStringList &output, const QString &name, const QString &type, const QString &null);
		void dumpVersions (QStringList &output);
};

#endif
