#ifndef _adminFunctions_h
#define _adminFunctions_h

#include "src/db/Database.h"

void initialize_database (Database &root_db, QString server, int port, QString rootName, QString rootPassword) throw (Database::ex_init_failed, Database::ex_access_denied, Database::ex_allocation_error, Database::ex_connection_failed, Database::ex_parameter_error);

#endif

