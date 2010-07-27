#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <QString>
#include <QStringList>

QString getEnvironmentVariable (const QString& name);
QStringList getSystemPath (const QString &environmentVariable="PATH");

#endif
