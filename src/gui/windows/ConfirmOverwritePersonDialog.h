#ifndef CONFIRMOVERWRITEPERSONDIALOG_H
#define CONFIRMOVERWRITEPERSONDIALOG_H

#include "ui_ConfirmOverwritePersonDialog.h"

#include <QDialog>

class Person;
template<class T> class QList;
template<class T> class ObjectModel;
template<class T> class ObjectListModel;
template<class T> class MutableObjectList;

/**
 * A window to let the user confirm the merging of people
 *
 * The window displays the wrong entries and the correct one and informs the
 * user about the consequences of merging. The user can confirm or cancel the
 * process.
 */
class ConfirmOverwritePersonDialog : public QDialog
{
	public:
		ConfirmOverwritePersonDialog (QWidget *parent=NULL, Qt::WindowFlags f=NULL);
		~ConfirmOverwritePersonDialog ();

		static bool confirmOverwrite (const Person &correctPerson, const QList<Person> &wrongPeople, QWidget *parent=NULL);

	protected:
		void setup (const Person &correctPerson, const QList<Person> &wrongPeople);

	private:
		Ui::ConfirmOverwritePersonDialogClass ui;
};

#endif
