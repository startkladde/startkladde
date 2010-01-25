#ifndef _adminFunctions_h
#define _adminFunctions_h

#include "src/db/OldDatabase.h"

void initialize_database (OldDatabase &root_db, QString server, int port, QString rootName, QString rootPassword) throw (OldDatabase::ex_init_failed, OldDatabase::ex_access_denied, OldDatabase::ex_allocation_error, OldDatabase::ex_connection_failed, OldDatabase::ex_parameter_error);

#endif

