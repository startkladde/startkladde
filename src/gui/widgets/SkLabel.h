#ifndef _SkLabel_h
#define _SkLabel_h

/*
 * SkLabel
 * martin
 * 2005-02-04
 */
#include <QLabel>
#include <QColor>

class SkLabel:public QLabel
	// Not hidden by hide () because this messes up the QLayout.
{
	Q_OBJECT

	public:
		SkLabel (QColor _background_color, QColor _error_color, QWidget *parent=NULL, const char *name=NULL);
		SkLabel (const QString &text, QWidget *parent=NULL, const char *name=NULL);

	public slots:
		void set_error (bool _error);
		void set_invisible (bool _invisible);

	signals:
		void clicked ();

	protected:
		void set_colors ();
		// TODO implement this so these functions can be used to set the
		// foreground/normal background colors.
		// The inactive palette state might also be used to hide the widget.
		//void paletteChange (const QPalette &oldPalette)
		virtual void mouseDoubleClickEvent (QMouseEvent *e);

	private:
		QColor background_color;
		QColor error_color;
		bool invisible;
		bool error;
};

#endif

