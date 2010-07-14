#include "ObjectSelectWindow.h"

#include "src/gui/widgets/SkTreeWidgetItem.h"
//#include "src/logging/messages.h"
#include "src/text.h"

template<class T> ObjectSelectWindow<T>::ObjectSelectWindow (const QList<T> &objects, dbId selectedId, QWidget *parent):
	ObjectSelectWindowBase (parent)
{
	int i=0;
	QString title;

	QStringList header;

	// TODO improve: ObjectModel? StringList?
	while (title=T::get_selector_caption (i), !title.isEmpty ())
	{
		header.append (title);
		++i;
	}

	ui.objectList->setColumnCount (header.size ());
	ui.objectList->setHeaderLabels (header);


	unknownItem=new SkTreeWidgetItem (ui.objectList, "(Unbekannt)");
	unknownItem->setFirstColumnSpanned (true);

	newItem=new SkTreeWidgetItem (ui.objectList, "(Neu anlegen)");
	newItem->setFirstColumnSpanned (true);

	ui.objectList->setCurrentItem (unknownItem);

	int numColumns=ui.objectList->columnCount ();
	foreach (const T &object, objects)
	{
		SkTreeWidgetItem *item=new SkTreeWidgetItem (ui.objectList);
		item->id=object.getId ();

		for (int i=0; i<numColumns; ++i)
			item->setText (i, object.get_selector_value (i));

		if (!idInvalid (object.getId ()) && object.getId ()==selectedId)
			ui.objectList->setCurrentItem (item);
	}

	// Resize all columns to their contents
	for (int i=0; i<numColumns; ++i)
		ui.objectList->resizeColumnToContents (i);
}

template<class T> ObjectSelectWindow<T>::~ObjectSelectWindow ()
{
}

template<class T> ObjectSelectWindowBase::Result ObjectSelectWindow<T>::select
	(dbId *resultId, const QString &title, const QString &text, const QList<T> &objects, dbId preselectionId, QWidget *parent)
{
	ObjectSelectWindow<T> window (objects, preselectionId, parent);

	window.setWindowTitle (title);

	window.ui.textLabel->setText (text);
	if (isBlank (text)) window.ui.textLabel->setVisible (false);


	int result=window.exec ();

	if (result==QDialog::Rejected)
	{
		return resultCancelled;
	}
	else
	{
		QList<QTreeWidgetItem *> selected=window.ui.objectList->selectedItems ();
		if (selected.empty ())
			return resultNoneSelected;
		else if (selected[0]==window.newItem)
			return resultNew;
		else if (selected[0]==window.unknownItem)
			return resultUnknown;
		else
		{
			// There may only be sk_list_view_items in the list.
			if (resultId) *resultId=(dynamic_cast<SkTreeWidgetItem *> (selected[0]))->id;
			return resultOk;
		}
	}
}


// Instantiate the class templates
#include "src/model/Person.h"

template class ObjectSelectWindow<Person>;
