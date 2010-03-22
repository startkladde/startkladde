#ifndef LAUNCHMETHODEDITORPANE_H_
#define LAUNCHMETHODEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_LaunchMethodEditorPane.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode

class LaunchMethod;

class LaunchMethodEditorPane: public ObjectEditorPane<LaunchMethod>
{
    Q_OBJECT

	public:
		LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent=NULL);
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
