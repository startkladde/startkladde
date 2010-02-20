#ifndef _SkLabel_h
#define _SkLabel_h

#include <QLabel>
#include <QColor>

#include "src/accessor.h"

/**
 * A QLabel which can be made invisible without changing the layout by setting
 * the foreground color equal to the background color. This is called
 * "concealing" the label.
 */
class SkLabel: public QLabel
	// Not hidden by hide () because this messes up the QLayout.
{
	Q_OBJECT

	public:
		// Construction
		SkLabel (QWidget *parent=0, Qt::WindowFlags f=0);
		SkLabel (const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);

		// Property access
		void setDefaultBackgroundColor (const QColor &color);
		attr_accessor (QColor, ErrorColor, errorColor);
		void resetDefaultBackgroundColor ();
		QColor getDefaultBackgroundColor ();

	public slots:
		void setConcealed (bool concealed);
		void setError (bool error);
		void setNumber (int number);
		void setNumber (float number);
		void setNumber (double number);

	signals:
		void doubleClicked (QMouseEvent *event);

	protected:
		void updateColors ();
		virtual void mouseDoubleClickEvent (QMouseEvent *event) { emit doubleClicked (event); }

	private:
		bool concealed;
		bool error;

		QColor defaultBackgroundColor;
		bool useDefaultBackgroundColor;
		QColor errorColor;
};

#endif

