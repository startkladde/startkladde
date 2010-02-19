#ifndef OBJECTLISTWINDOW_H
#define OBJECTLISTWINDOW_H

#include <QtGui/QMainWindow>

#include "ObjectListWindowBase.h"
#include "src/gui/windows/objectEditor/ObjectEditorPane.h"
#include "src/model/objectList/ObjectListModel.h"

class DataStorage;
class QSortFilterProxyModel;

template <class T> class ObjectListWindow: public ObjectListWindowBase
{
	public:
		ObjectListWindow (DataStorage &dataStorage, ObjectListModel<T> *list, bool listOwned, QWidget *parent=NULL);
		~ObjectListWindow();

		virtual void on_actionNew_triggered ();
		virtual void on_actionEdit_triggered ();
		virtual void on_actionDelete_triggered ();
		virtual void on_actionRefresh_triggered ();

		virtual void on_table_activated (const QModelIndex &index);

	private:
		DataStorage &dataStorage;
		ObjectListModel<T> *list;
		bool listOwned;
		QSortFilterProxyModel *proxyModel;
};

#endif // OBJECTLISTWINDOW_H
