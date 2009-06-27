#ifndef _dbProxy_h
#define _dbProxy_h

#include <string>

#include <QDateTime>
#include <QString>
#include <QComboBox>

#include "src/dataTypes.h"
#include "src/db/Database.h"
#include "src/logging/messages.h"
#include "src/model/Plane.h"
#include "src/model/LaunchType.h"


int schleppflugzeug_aus_startart (Database *db, Plane *fz, int  sa_num);
db_id schleppmaschine_flog (Database *db, db_id sa_id, Time t);

#endif

