#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <QString>
#include <QStringList>

#include "src/notr.h"

QString getEnvironmentVariable (const QString& name);
QStringList getSystemPath (const QString &environmentVariable=notr ("PATH"));

#endif
