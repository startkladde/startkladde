#ifndef db_proxy_h
#define db_proxy_h

#include <string>

#include <QDateTime>
#include <QString>
#include <QComboBox>

#include "src/data_types.h"
#include "src/db/sk_db.h"
#include "src/logging/messages.h"
#include "src/model/Plane.h"
#include "src/model/LaunchType.h"

using namespace std;

int schleppflugzeug_aus_startart (sk_db *db, Plane *fz, int  sa_num);
db_id schleppmaschine_flog (sk_db *db, db_id sa_id, sk_time_t t);

#endif

