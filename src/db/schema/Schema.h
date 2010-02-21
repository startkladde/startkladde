#ifndef SCHEMA_H_
#define SCHEMA_H_

#include "src/db/migration/Migration.h"

#include <QList>

class Schema: public Migration
{
	public:
		Schema (Database &database);
		virtual ~Schema ();

		virtual QList<quint64> getVersions ()=0;
};

#endif
