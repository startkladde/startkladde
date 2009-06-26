#include "mime_header.h"

#include "src/text.h"

const QString mime_header::text_name_content_type="Content-Type";
const QString mime_header::text_name_content_disposition="Content-Disposition";


mime_header::mime_header (const QString &text, const QString &_name)
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

	QStringList text_parts=text.split (';');
	trim (text_parts);

	// Save and remove the first QString which is either of form "name: value"
	// or "value".
	QString name_value=text_parts.front ();
	text_parts.pop_front ();

	// Determine the name
	if (_name.isEmpty ())
	{
		// name_value is of form "name: value"
		if (!name_value.contains (":")) return;
		split_string (name, value, ":", name_value);
		name=name.trimmed ();
		value=value.trimmed ();
	}
	else
	{
		name=_name;
		value=name_value.trimmed ();
	}

	QStringListIterator it (text_parts);
	while (it.hasNext ())
	{
		QString key, val;
		split_string (key, val, "=", it.next ());
		// Remove whitespace and ""
		key=key.trimmed();
		val=val.trimmed();
		if (val.length ()>1 && val.at (0)=='"' && val.at (val.length ()-1)=='"')
			val=val.mid (1, val.length ()-2);
		args.set_value (key, val);
	}
}

std::ostream &operator<< (std::ostream &s, const mime_header &mh)
{
	// TODO Escape (how to escape MIME headers?)
	s << mh.name << ": " << mh.value;

	QList<argument>::const_iterator end=mh.args.get_list ().end ();
	for (QList<argument>::const_iterator it=mh.args.get_list ().begin (); it!=end; ++it)
	{
		s << "; " << (*it).get_name () << "=\"" << (*it).get_value () << "\"";
	}

	return s;
}

void mime_header::test (const QString &text, const QString &_name)
{
	if (_name.isEmpty ())
		std::cout << "In:  [" << text << "]" << std::endl;
	else
		std::cout << "In:  [" << _name << "] [" << text << "]" << std::endl;

	std::cout << "Out: [" << mime_header (text, _name) << "]" << std::endl;
}

void mime_header::test ()
{
	test ("multipart/form-data; boundary=----------PmlqQ32HDjGJWAIJ7Ez1C", "Content-Type");
	test ("Content-Disposition: form-data; name=\"datei\"; filename=\"TODO\"");
	test ("Content-Type: application/octet-stream");
}

