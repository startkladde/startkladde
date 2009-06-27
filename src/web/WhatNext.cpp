#include "WhatNext.h"

WhatNext WhatNext::go_on ()
{
	return WhatNext (wn_go_on);
}

WhatNext WhatNext::output_error (const QString &message, const QString &explanation)
{
	return WhatNext (wn_output_error)
		.set_data (message)
		.set_description (explanation)
		;
}

WhatNext WhatNext::output_document ()
{
	return WhatNext (wn_output_document);
}

WhatNext WhatNext::output_raw_document (const QString &document, const QString &mime_type, const QString &filename, const QString &description)
{
	return WhatNext (wn_output_raw_document)
		.set_data (document)
		.set_mime_type (mime_type)
		.set_filename (filename)
		.set_description (description)
		;
}

WhatNext WhatNext::go_to_state (const QString &new_state_label, const QString &_message, bool _message_error)
{
	return WhatNext (wn_change_state)
		.set_state_label (new_state_label)
		.set_data (_message)
		.set_flag (_message_error);
}

WhatNext WhatNext::do_redirect (const QString &url)
{
	return WhatNext (wn_do_redirect)
		.set_data (url);
}

WhatNext WhatNext::end_program ()
{
	return WhatNext (wn_end_program);
}



