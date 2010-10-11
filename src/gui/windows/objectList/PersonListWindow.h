#ifndef PERSONLISTWINDOW_H_
#define PERSONLISTWINDOW_H_

#include "src/gui/windows/objectList/ObjectListWindow.h"

#include "src/model/Person.h"

class QAction;

class DbManager;

class PersonListWindow: public ObjectListWindow<Person>
{
	Q_OBJECT

	public:
		PersonListWindow (DbManager &manager, QWidget *parent=NULL);
		virtual ~PersonListWindow ();

	protected:
		virtual void prepareContextMenu (QMenu *contextMenu);

	protected slots:
		void mergeAction_triggered ();

	private:
		QAction *mergeAction;
};

#endif
