#ifndef OBJECTLISTWINDOW_H
#define OBJECTLISTWINDOW_H

#include <QtGui/QMainWindow>

#include "ObjectListWindowBase.h"

class DbManager;
class QSortFilterProxyModel;
template<class T> class ObjectListModel;

template <class T> class ObjectListWindow: public ObjectListWindowBase
{
	public:
		ObjectListWindow (DbManager &manager, ObjectListModel<T> *list, bool listOwned, QWidget *parent=NULL);
		~ObjectListWindow();

		virtual void on_actionNew_triggered ();
		virtual void on_actionEdit_triggered ();
		virtual void on_actionDelete_triggered ();
		virtual void on_actionRefresh_triggered ();

		virtual void on_table_activated (const QModelIndex &index);

	private:
		ObjectListModel<T> *list;
		bool listOwned;
		QSortFilterProxyModel *proxyModel;
};

#endif // OBJECTLISTWINDOW_H
