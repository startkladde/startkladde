#ifndef TEXT_H_
#define TEXT_H_

#include <QSet>
#include <QDateTime>
#include <QString>

class QAbstractButton;

#define STRINGIFY_INDIR(arg) #arg
#define STRINGIFY(arg) STRINGIFY_INDIR(arg)

extern const QString whitespace;

bool isBlank (const QString &string);
bool isNone (const QString &eintrag);
bool isNone (const QString &eintrag1, const QString &eintrag2);

void trim (QStringList &strings);

QString firstToUpper (const QString &text);
QString firstToLower (const QString &text);
QString capitalize (const QString &string);

QString countText (int count, const QString &singular, const QString &plural);

QString simplifyClubName (const QString &clubName);
bool locationEntryCanBeChanged (const QString &location);

QString insertMnemonic (const QString &text, const QString &disallowed, bool fallbackToFirst=false);
QChar getMnemonic (const QString &text);
QChar getMnemonic (const QAbstractButton *button);

#endif

