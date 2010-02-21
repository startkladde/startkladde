#ifndef PERSONEDITORPANE_H_
#define PERSONEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_PersonEditorPane.h"

#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode

class Person;
class DataStorage;

class PersonEditorPane: public ObjectEditorPane<Person>
{
    Q_OBJECT

	public:
		PersonEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);
		virtual ~PersonEditorPane();

		virtual void objectToFields (const Person &person);
		virtual Person determineObject ();

	protected:
		virtual void fillData ();

	private:
		Ui::PersonEditorPaneClass ui;
};

#endif // PERSONEDITORPANE_H
