#ifndef PLANEEDITORPANE_H
#define PLANEEDITORPANE_H

#include "ObjectEditorPane.h"
#include "ui_PlaneEditorPane.h"

// TODO reduce dependencies
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h"

class Plane;

class PlaneEditorPane: public ObjectEditorPane<Plane>
{
    Q_OBJECT

	public:
		PlaneEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);
		virtual ~PlaneEditorPane();

		virtual void objectToFields (const Plane &plane);
		virtual Plane determineObject ();

	public slots:
		virtual void on_registrationInput_editingFinished ();

	protected:
		virtual void fillData ();

	private:
		Ui::PlaneEditorPaneClass ui;
};


#endif // PLANEEDITORPANE_H
