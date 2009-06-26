#ifndef text_h
#define text_h

#include <sstream>

#include <QSet>
#include <QDateTime>
#include <QMessageBox>
#include <QString>

#include "src/data_types.h"
#include "src/db/db_types.h"

extern const QString whitespace;

bool eintrag_ist_leer (QString eintrag);
bool check_message (QWidget *, const QString &);
QString t_pilot_bezeichnung (flug_typ, casus c=cas_nominativ);
QString t_begleiter_bezeichnung (flug_typ, casus c=cas_nominativ);
void replace_substring (QString &, const QString&, const QString&);
void replace_tabs (QString &s);
QString simplify_club_name (const QString s);
bool club_name_identical (const QString s1, const QString s2);
QString concatenate_comments (const QString& s1, const QString& s2);
QString string_or_none (const QString& text);
bool parameter_matches (char *parameter, char *val1, char *val2);
void trim (QStringList &strings);
bool is_date (const QString& s);
QString hex_string (unsigned char c);
QString latex_escape (const QString& o);
QString csv_escape (const QString& o);
QString get_environment (const QString& name);
QString cgi_escape (const QString &text, bool leave_high=false);
QString cgi_unescape (const QString &text);
QString html_escape (const QString& s, bool also_escape_newlines=true);
QStringList html_escape (const QStringList &list, bool also_escape_newlines=true);
QString bool_to_string (bool val, const QString &true_value="Ja", const QString &false_value="Nein");
bool string_to_bool (const QString &text);
QString make_string (const QSet<QString> s, const QString &separator=",");
void split_string (QString &string1, QString &string2, QString separator, QString text);

std::ostream &operator<< (std::ostream &s, const QString &c);

#endif

