#ifndef db_proxy_h
#define db_proxy_h

#include <string>

#include <qdatetime.h>
#include <qstring.h>
#include <qcombobox.h>

#include "data_types.h"
#include "messages.h"
#include "sk_flugzeug.h"
#include "startart_t.h"

#include "sk_db.h"

using namespace std;

int schleppflugzeug_aus_startart (sk_db *db, sk_flugzeug *fz, int  sa_num);
db_id schleppmaschine_flog (sk_db *db, db_id sa_id, sk_time_t t);

#endif

