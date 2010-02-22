#ifndef SCHEMADUMPER_H_
#define SCHEMADUMPER_H_

#include <QString>

class DatabaseInterface;

class SchemaDumper
{
	public:
		SchemaDumper (DatabaseInterface &databaseInterface);
		virtual ~SchemaDumper ();

		QString dumpSchema ();
		void dumpSchemaToFile (const QString &filename);

	protected:
		DatabaseInterface &databaseInterface;

		void dumpTables   (QStringList &output);
		void dumpTable    (QStringList &output, const QString &name);
		void dumpColumns  (QStringList &output, const QString &table);
		void dumpColumn   (QStringList &output, const QString &name, const QString &type, const QString &null);
		void dumpVersions (QStringList &output);
};

#endif
