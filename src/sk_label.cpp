#include "sk_label.h"

sk_label::sk_label (QColor _background_color, QColor _error_color, QWidget *parent, const char *name)/*{{{*/
	:QLabel (parent, name), background_color (_background_color), error_color (_error_color), invisible (false), error (false)
{
}/*}}}*/

sk_label::sk_label (const QString &text, QWidget *parent, const char *name)/*{{{*/
	:QLabel (text, parent, name)
{
}
/*}}}*/

void sk_label::set_error (bool _error)/*{{{*/
{
	error=_error;
	set_colors ();
}
/*}}}*/

void sk_label::set_invisible (bool _invisible)/*{{{*/
{
	invisible=_invisible;
	set_colors ();
}
/*}}}*/

void sk_label::set_colors ()/*{{{*/
{
	if (invisible)
	{
		setPalette (parentWidget ()->palette ());
		setAutoFillBackground (true);

		QPalette pal=palette ();
		QColor color=parentWidget ()->palette ().color (QPalette::Background);
		pal.setColor (QPalette::Foreground, color);
		pal.setColor (QPalette::Background, color);

		setPalette (pal);
	}
	else
	{
		setAutoFillBackground (true);

		QPalette pal=palette ();
		pal.setColor (QPalette::Foreground, QColor (0, 0, 0));

		if (error)
			pal.setColor (QPalette::Background, error_color);
		else
			pal.setColor (QPalette::Background, background_color);

		setPalette (pal);
	}
}
/*}}}*/

void sk_label::mouseDoubleClickEvent (QMouseEvent *e)/*{{{*/
{
	emit (clicked ());
}
/*}}}*/

