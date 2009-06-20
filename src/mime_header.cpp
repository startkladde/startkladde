#include "mime_header.h"

#include "text.h"

const string mime_header::text_name_content_type="Content-Type";
const string mime_header::text_name_content_disposition="Content-Disposition";


mime_header::mime_header (const string &text, const string &_name)/*{{{*/
{
	// multipart/form-data; boundary=----------PmlqQ32HDjGJWAIJ7Ez1C
	//   name: (must be given)
	//   value: multipart/form-data
	//   args:
	//     - "boundary", "----------PmlqQ32HDjGJWAIJ7Ez1C"
	// Content-Disposition: form-data; name="datei"; filename="TODO"
	//   name: Content-Disposition
	//   value: form-data
	//   args:
	//     - "name", "datei"
	//     - "filename", "TODO"
	// Content-Type: application/octet-stream
	//   name: Content-Type
	//   value: application/octet-stream
	//   args: empty

	list<string> text_parts;
	split_string (text_parts, ";", text);
	trim (text_parts);

	// Save and remove the first string which is either of form "name: value"
	// or "value".
	string name_value=text_parts.front ();
	text_parts.pop_front ();
	
	// Determine the name
	if (_name.empty ())
	{
		// name_value is of form "name: value"
		if (name_value.find (":")==string::npos) return;
		split_string (name, value, ":", name_value);
		name=trim (name);
		value=trim (value);
	}
	else
	{
		name=_name;
		value=trim (name_value);
	}

	list<string>::const_iterator end=text_parts.end ();
	for (list<string>::const_iterator it=text_parts.begin (); it!=end; ++it)
	{
		string key, val;
		split_string (key, val, "=", *it);
		// Remove whitespace and ""
		key=trim (key);
		val=trim (val);
		if (val.length ()>1 && val.at (0)=='"' && val.at (val.length ()-1)=='"')
			val=val.substr (1, val.length ()-2);
		args.set_value (key, val);
	}
}/*}}}*/

ostream &operator<< (ostream &s, const mime_header &mh)/*{{{*/
{
	// TODO Escape (how to escape MIME headers?)
	s << mh.name << ": " << mh.value;

	list<argument>::const_iterator end=mh.args.get_list ().end ();
	for (list<argument>::const_iterator it=mh.args.get_list ().begin (); it!=end; ++it)
	{
		s << "; " << (*it).get_name () << "=\"" << (*it).get_value () << "\"";
	}

	return s;
}
/*}}}*/

void mime_header::test (const string &text, const string &_name)/*{{{*/
{
	if (_name.empty ())
		cout << "In:  [" << text << "]" << endl;
	else
		cout << "In:  [" << _name << "] [" << text << "]" << endl;

	cout << "Out: [" << mime_header (text, _name) << "]" << endl;
}
/*}}}*/

void mime_header::test ()/*{{{*/
{
	test ("multipart/form-data; boundary=----------PmlqQ32HDjGJWAIJ7Ez1C", "Content-Type");
	test ("Content-Disposition: form-data; name=\"datei\"; filename=\"TODO\"");
	test ("Content-Type: application/octet-stream");
}
/*}}}*/

