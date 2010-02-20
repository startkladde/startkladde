#ifndef _SkComboBox_h
#define _SkComboBox_h

#include <iostream>

#include <QComboBox>
#include <QString>

#include "src/accessor.h"

/**
 * A QComboBox with some additional functionality.
 *
 * A default prefix can be set. If the text starts with the default prefix,
 * only the part after the default prefix is selected when the widget receives
 * focus.
 */
/*
 * TODO:
 *   - Improve the completer behavior:
 *       - The text should be completed (InlineCompletion, so we can simply tab
 *         on) *and* a list should be shown (PopupCompletion, so we can mouse-
 *         select a value)
 *       - When starting to type in lower case and the completer matches an
 *         upper case completion, the text in the lineEdit should be replaced
 *         with the upper case text.
 *     Issues with that:
 *       - QCompleter does not seem to support InlineCompletion and
 *         PopupCompletion at the same time
 *       - Potentially, the completer cannot change the text already typed and
 *         SkComboBox also has to be changed.
 *       - Setting the Completer in the SkComboBox constructor does not seem to
 *         work; also, completer() is NULL here although it is !=NULL later. The
 *         default completer seems to be installed somewhere else.
 *         => maybe use the Polish event (event() function)
 */
class SkComboBox:public QComboBox
{
	Q_OBJECT

	public:
		SkComboBox (QWidget *parent=0);

//		void setLineEdit (QLineEdit *edit);
		attr_accessor (QString, DefaultPrefix, defaultPrefix);

		bool setCurrentItemByItemData (QVariant itemData);
		QVariant currentItemData (int role=Qt::UserRole);

		void setEditable (bool editable);

	signals:
		/**
		 * Emitted when the text is changed by user interaction, either by
		 *editing it or by selecting an item from the list.
		 */
		void textEdited (const QString &text);

		/**
		 * Emitted when Enter is pressed, the line edit loses focus or the
		 * user selects a value from the list.
		 *
		 * Note that the line edit will also lose focus when the combo box is
		 * opened (it should be "the ComboBox loses focus" instead of "the line
		 * edit loses focus" - Enter would have to be handled separately).
		 */
		void editingFinished (const QString &text);

	protected slots:
		void lineEdit_editingFinished () { emit editingFinished (currentText ()); }

	protected:
		virtual void focusInEvent (QFocusEvent *event);

	private:
		QString defaultPrefix;
};

#endif

