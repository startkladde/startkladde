#ifndef CHOICE_DIALOG_H
#define CHOICE_DIALOG_H

#include "src/gui/SkDialog.h"
#include "ui_ChoiceDialog.h"

#include <QRadioButton>
#include <QList>

/**
 * A dialog that lets the user choose one of several options
 *
 * Example use:
 *     ChoiceDialog dialog (parent);
 *
 *     dialog.addOption ("&Foo");    // Add some options
 *     dialog.addOption ("&Bar");
 *     dialog.addOption ("B&az");
 *     dialog.setSelectedOption (0); // Select the first option
 *
 *     int result=dialog.exec ();    // Show the dialog modally
 *     dialog.getSelectedOption ();  // Get the selected option
 */
class ChoiceDialog: public SkDialog<Ui::ChoiceDialogClass>
{
	public:
		ChoiceDialog (QWidget *parent = 0, Qt::WindowFlags f=0);
		~ChoiceDialog();

		static int choose (QWidget *parent, const QStringList &options, int defaultIndex=0, Qt::WindowFlags flags=0);

		void addOption (const QString &text);

		void setSelectedOption (int option);
		int getSelectedOption ();

	protected:
		QRadioButton *addRadioButton ();

		QList<QRadioButton *> radioButtons;
};

#endif
