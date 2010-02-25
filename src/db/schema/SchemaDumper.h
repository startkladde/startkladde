#ifndef SCHEMADUMPER_H_
#define SCHEMADUMPER_H_

#include <QString>

namespace Db { namespace Interface { class Interface; } }

class SchemaDumper
{
	public:
		SchemaDumper (Db::Interface::Interface &interface);
		virtual ~SchemaDumper ();

		QString dumpSchema ();
		void dumpSchemaToFile (const QString &filename);

	protected:
		Db::Interface::Interface &interface;

		void dumpTables   (QStringList &output);
		void dumpTable    (QStringList &output, const QString &name);
		void dumpColumns  (QStringList &output, const QString &table);
		void dumpColumn   (QStringList &output, const QString &name, const QString &type, const QString &null);
		void dumpVersions (QStringList &output);
};

#endif
