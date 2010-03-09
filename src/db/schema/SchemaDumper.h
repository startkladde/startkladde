#ifndef SCHEMADUMPER_H_
#define SCHEMADUMPER_H_

#include <QString>

class Interface;

class SchemaDumper
{
	public:
		SchemaDumper (Interface &interface);
		virtual ~SchemaDumper ();

		QString dumpSchema ();
		void dumpSchemaToFile (const QString &filename);

	protected:
		Interface &interface;

		void dumpTables   (QStringList &output);
		void dumpTable    (QStringList &output, const QString &name);
		void dumpColumns  (QStringList &output, const QString &table);
		void dumpColumn   (QStringList &output, const QString &name, const QString &type, const QString &null);
		void dumpVersions (QStringList &output);
};

#endif
