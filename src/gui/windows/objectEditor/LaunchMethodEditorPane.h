#ifndef _LaunchMethodEditorPane_h
#define _LaunchMethodEditorPane_h

#include "ObjectEditorPane.h"
#include "ui_LaunchMethodEditorPane.h"

// TODO reduce dependencies
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h"

class LaunchMethod;

class LaunchMethodEditorPane: public ObjectEditorPane<LaunchMethod>
{
    Q_OBJECT

	public:
		LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, DataStorage &dataStorage, QWidget *parent=NULL);
		virtual ~LaunchMethodEditorPane();

		virtual void objectToFields (const LaunchMethod &launchMethod);
		virtual LaunchMethod determineObject ();

	public slots:
		virtual void on_typeInput_activated (int index);

	protected:
		virtual void fillData ();

	private:
		Ui::LaunchMethodEditorPaneClass ui;
};


#endif // PLANEEDITORPANE_H
