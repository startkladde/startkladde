#ifndef admin_functions_h
#define admin_functions_h

#include <string>

#include "src/db/sk_db.h"

using namespace std;

void initialize_database (sk_db &admin_db) throw (sk_db::ex_init_failed, sk_db::ex_access_denied, sk_db::ex_allocation_error, sk_db::ex_connection_failed, sk_db::ex_parameter_error);

#endif

