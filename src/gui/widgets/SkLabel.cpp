#include "SkLabel.h"

#include <iostream>

#include "src/text.h"

// ******************
// ** Construction **
// ******************

SkLabel::SkLabel (QWidget *parent, Qt::WindowFlags f):
	QLabel (parent, f),
	concealed (false),
	error (false),
	defaultBackgroundColor (255, 255, 255),
	useDefaultBackgroundColor (false),
	errorColor (255, 0, 0)
{
}

SkLabel::SkLabel (const QString &text, QWidget *parent, Qt::WindowFlags f):
	QLabel (text, parent, f),
	concealed (false),
	error (false),
	defaultBackgroundColor (255, 255, 255),
	useDefaultBackgroundColor (false),
	errorColor (255, 0, 0)
{
}


// *********************
// ** Property access **
// *********************

void SkLabel::setDefaultBackgroundColor (const QColor &color)
{
	defaultBackgroundColor=color;
	useDefaultBackgroundColor=true;

	updateColors ();
}

void SkLabel::resetDefaultBackgroundColor ()
{
	useDefaultBackgroundColor=false;

	updateColors ();
}

QColor SkLabel::getDefaultBackgroundColor ()
{
	return defaultBackgroundColor;
}

void SkLabel::setConcealed (bool concealed)
{
	this->concealed=concealed;
	updateColors ();
}

void SkLabel::setError (bool error)
{
	this->error=error;
	updateColors ();
}



// ******************
// ** State update **
// ******************

void SkLabel::updateColors ()
{
	QPalette p=palette ();

	// TODO implement error state display
	if (concealed)
	{
		// Concealed => foreground and background like parent background
		setAutoFillBackground (true);
		p.setColor (QPalette::Foreground, parentWidget ()->backgroundColor ());
		p.setColor (QPalette::Background, parentWidget ()->backgroundColor ());
	}
	else if (error)
	{
		// Error => given error background, same foreground as parent
		setAutoFillBackground (true);
		p.setColor (QPalette::Foreground, parentWidget ()->foregroundColor ());
		p.setColor (QPalette::Background, errorColor);
	}
	else if (useDefaultBackgroundColor)
	{
		// Not concealed => given background, same foreground as parent
		setAutoFillBackground (true);
		p.setColor (QPalette::Foreground, parentWidget ()->foregroundColor ());
		p.setColor (QPalette::Background, defaultBackgroundColor);
	}
	else
	{
		// Not concealed, background color not used => same palette as parent
		setAutoFillBackground (false);
		p.setColor (QPalette::Foreground, parentWidget ()->foregroundColor ());
		p.setColor (QPalette::Background, parentWidget ()->backgroundColor ());
	}

	setPalette (p);
}


// **************
// ** Contents **
// **************

void SkLabel::setNumber (int number)
{
	setText (QString::number (number));
}

void SkLabel::setNumber (float number)
{
	setText (QString::number (number));
}

void SkLabel::setNumber (double number)
{
	setText (QString::number (number));
}

// **********
// ** Misc **
// **********



//void SkLabel::set_error (bool _error)
//{
//	error=_error;
//	set_colors ();
//}

