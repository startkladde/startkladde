#include "SchemaDumper.h"

#include <iostream>

#include <QFile>
#include <QDateTime>
#include <QStringList>

#include "src/db/interface/Interface.h"
#include "src/db/migration/Migrator.h" // Required for migrationsTableName/migrationsColumnName
#include "src/db/result/Result.h"
#include "src/text.h"
#include "src/notr.h"

SchemaDumper::SchemaDumper (Interface &interface):
	interface (interface)
{
}

SchemaDumper::~SchemaDumper ()
{
}

QString SchemaDumper::dumpSchema ()
{
	QStringList output;

	output << notr ("---");

	dumpTables (output);
	dumpVersions (output);

	return output.join (notr ("\n"));
}

void SchemaDumper::dumpSchemaToFile (const QString &filename)
{
	QString dump=dumpSchema ();

	QFile file (filename);
	try
	{
		file.open (QFile::WriteOnly);

		QString generatedDate=QDateTime::currentDateTime ().toUTC ().toString (Qt::ISODate);
		file.write (qnotr ("# This file has been autogenerated by SchemaDumper on %1 UTC.\n").arg (generatedDate).toUtf8 ());
		file.write ( notr ("# It should not be modified as any changes will be overwritten.\n"));
		file.write ( notr ("#\n"));
		file.write ( notr ("# This file should be checked into version control. See the developer\n"));
		file.write ( notr ("# documentation (doc/internal/database.txt) for further information.\n"));

		file.write (dump.toUtf8 ());
		file.write (notr ("\n"));
		file.close ();
	}
	catch (...)
	{
		file.close ();
		throw;
	}
}

void SchemaDumper::dumpTables (QStringList &output)
{
	output << notr ("tables:");

	QSharedPointer<Result> result=interface.executeQueryResult (notr ("SHOW TABLES"));

	while (result->next ())
		dumpTable (output, result->value (0).toString ());
}

void SchemaDumper::dumpTable (QStringList &output, const QString &name)
{
	// Don't dump the migrations table
	if (name==Migrator::migrationsTableName) return;

	output << qnotr ("- name: \"%1\"").arg (name);
	dumpColumns (output, name);
	dumpIndexes (output, name);
}
void SchemaDumper::dumpColumns (QStringList &output, const QString &table)
{
	output << notr ("  columns:");

	Query query=Query (notr ("SHOW FULL COLUMNS FROM %1")).arg (table);
	QSharedPointer<Result> result=interface.executeQueryResult (query);

	QSqlRecord record=result->record ();
	// TODO: handle index not found. Also, this is backend specific and should be in Interface
	int nameIndex =record.indexOf (notr ("Field"));
	int typeIndex =record.indexOf (notr ("Type" ));
	int nullIndex =record.indexOf (notr ("Null" ));
	int keyIndex  =record.indexOf (notr ("Key"  ));
	int extraIndex=record.indexOf (notr ("Extra"));

	while (result->next ())
	{
		QString name=result->value (nameIndex).toString ();
		QString type=result->value (typeIndex).toString ();
		QString null=result->value (nullIndex).toString ();
		QString key=result->value (keyIndex).toString ();
		QString extra=result->value (extraIndex).toString ();

		dumpColumn (output, name, type, null, key, extra);
	}
}

void SchemaDumper::dumpColumn (QStringList &output, const QString &name, const QString &type, const QString &null, const QString &key, const QString &extra)
{
	output << qnotr ("  - name: \"%1\"").arg (name);
	output << qnotr ("    type: \"%1\"").arg (type);
	output << qnotr ("    nullok: \"%1\"").arg (null);
	if (key==notr ("PRI"))
		output << qnotr ("    primary_key: true");
	if (!isBlank (extra))
		output << qnotr ("    extra: \"%1\"").arg (extra);
}

void SchemaDumper::dumpIndexes (QStringList &output, const QString &table)
{
	QList<IndexSpec> indexes=interface.showIndexes (table);

	// Don't output anything if there are no indexes
	if (indexes.isEmpty ()) return;

	output << notr ("  indexes:");
	foreach (const IndexSpec &index, indexes)
		dumpIndex (output, index);
}

void SchemaDumper::dumpIndex (QStringList &output, const IndexSpec &index)
{
	output << qnotr ("  - name: \"%1\"").arg (index.getName ());
	output << qnotr ("    columns: \"%1\"").arg (index.getColumns ());
}

void SchemaDumper::dumpVersions (QStringList &output)
{
	// TODO handle empty/nonexistant migrations table
	output << notr ("versions:");

	QString table=Migrator::migrationsTableName;
	QString column=Migrator::migrationsColumnName;

	Query query=Query::selectDistinctColumns (table, column);
	QSharedPointer<Result> result=interface.executeQueryResult (query);

	while (result->next ())
		output << qnotr ("- %1").arg (result->value (0).toString ());
}
