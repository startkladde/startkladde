#ifndef LAUNCHMETHODEDITORPANE_H_
#define LAUNCHMETHODEDITORPANE_H_

#include "ObjectEditorPane.h"
#include "ui_LaunchMethodEditorPane.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindowBase.h" // Required for ObjectEditorWindowBase::Mode
#include "src/model/LaunchMethod.h"

class LaunchMethodEditorPane: public ObjectEditorPane<LaunchMethod>
{
    Q_OBJECT

	public:
		LaunchMethodEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent=NULL);
		virtual ~LaunchMethodEditorPane();

		virtual void objectToFields (const LaunchMethod &launchMethod);
		virtual void fieldsToObject (LaunchMethod &object);

	public slots:
		virtual void on_typeInput_activated (int index);

	protected:
		virtual void loadData ();
		virtual void setupText ();
		virtual void changeEvent (QEvent *event);

	private:
		Ui::LaunchMethodEditorPaneClass ui;
};


#endif // PLANEEDITORPANE_H
