#ifndef text_h
#define text_h

//#undef QT_NO_MESSAGEBOX	// HUH?
//#include <qt.h>
//#include <qdialog.h>
#include <qmessagebox.h>

#include <sstream>
#include <string>
#include <set>

#include <qdatetime.h>
#include <qstring.h>

#include "data_types.h"
#include "db_types.h"

using namespace std;

extern const string whitespace;

bool eintrag_ist_leer (const QString &eintrag);
bool eintrag_ist_leer (const string &eintrag);
bool check_message (QWidget *, const string &);
string t_pilot_bezeichnung (flug_typ, casus c=cas_nominativ);
string t_begleiter_bezeichnung (flug_typ, casus c=cas_nominativ);
string trim (const string &);
void replace_substring (string &, const string&, const string&);
void replace_tabs (string &s);
string num_to_string (int num, unsigned int min_length=0, char pad='0'); 
string num_to_string (unsigned int num); 
string num_to_string (float num); 
string num_to_string (db_id num); 
//string num_to_string (size_t num);
QString simplify_club_name (const QString& s);
string simplify_club_name (const string& s);
bool club_name_identical (const string& s1, const string& s2);
string concatenate_comments (const string& s1, const string& s2);
string string_or_none (const string& text);
bool parameter_matches (char *parameter, char *val1, char *val2);
void split_string (list<string> &strings, const string &separators, const string &text);
void split_string (string &string1, string &string2, const string &separators, const string &text);
void trim (list<string> &strings);
bool is_date (const string& s);
string hex_string (unsigned char c);
string latex_escape (const string& o);
string csv_escape (const string& o);
string get_environment (const string& name);
string cgi_escape (const string &text, bool leave_high=false);
string cgi_unescape (const string &text);
string html_escape (const string& s, bool also_escape_newlines=true);
list<string> html_escape (const list<string> &list, bool also_escape_newlines=true);
string bool_to_string (bool val, const string &true_value="Ja", const string &false_value="Nein");
bool string_to_bool (const string &text);
bool starts_with (const string &text, const string &substring);
bool ends_with (const string &text, const string &substring);
string make_string (const set<string> s, const string &separator=",");

#endif

