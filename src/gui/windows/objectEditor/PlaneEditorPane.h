#ifndef PLANEEDITORPANE_H_
#define PLANEEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_PlaneEditorPane.h"

#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode

class Plane;

class PlaneEditorPane: public ObjectEditorPane<Plane>
{
    Q_OBJECT

	public:
		PlaneEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent=NULL);
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
