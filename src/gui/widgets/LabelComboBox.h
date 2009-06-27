#ifndef _LabelComboBox_h
#define _LabelComboBox_h

#include <QString>

#include <QFrame>
#include <QLabel>
#include <QStringList> // XXX

#include "src/dataTypes.h"
#include "src/gui/widgets/SkComboBox.h"


class lbl_cbox:public QFrame
{
	Q_OBJECT

	public:
		lbl_cbox (const char *text, bool editable, QWidget *parent, const char *name);
		QString currentText () const;
		virtual void setCurrentText (const QString & );
		void setAutoCompletion (bool a);
//		bool eventFilter (QObject *o, QEvent *e);
		QString current_text_string ();
		void fillStringArray (const QStringList& array, bool clear_first=true, bool blank_line=false, bool del=true);
		void fillFullStringArray (const QStringList& array, bool clear_first=true, bool blank_line=false, bool del=true);
		void setEditText (const QString &newText);
		QString edit_text_string ();
		void use_full (bool full);
		void insert_full_if_new (const QString &);
		void select_all ();
		int get_label_width () { return lbl->width (); }
		void set_label_width (int w) { lbl->setFixedWidth (w); }

	public slots:
		void clear ();

	signals:
		void activated (const QString &QString);
		void highlighted (const QString &QString);
		void focus_out ();	// The focus was moved out of the widget.
		void progress (int, int);

	protected:
//		virtual void resizeEvent (QResizeEvent *e);

	private slots:
		void some_focus_out ();

	private:
		bool focus_out_locked;
		QLabel *lbl;
		SkComboBox *cbox;
		SkComboBox *full_cbox;
		SkComboBox *used_cbox;
		void fillStringArray (SkComboBox *box, const QStringList& array, bool clear_first=true, bool blank_line=false, bool del=true);
		void set_active_cbox (SkComboBox *);
};

#endif

