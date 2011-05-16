#ifndef PERSONEDITORPANE_H_
#define PERSONEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_PersonEditorPane.h"

#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode
#include "src/model/Person.h"

class PersonEditorPane: public ObjectEditorPane<Person>
{
	Q_OBJECT

	public:
		PersonEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent=NULL);
		virtual ~PersonEditorPane();

		virtual void objectToFields (const Person &person);
		virtual void fieldsToObject (Person &object);

		void setNameObject (const Person &nameObject);

	public slots:
		void on_medicalValidityUnknownCheckbox_toggled ();
		void on_checkMedicalInput_currentIndexChanged ();

	protected:
		virtual void fillData ();
		QDate getEffectiveMedicalValidity ();

	private:
		Ui::PersonEditorPaneClass ui;
};

#endif // PERSONEDITORPANE_H
