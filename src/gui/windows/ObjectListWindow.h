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
//		ObjectListWindow (DbManager &manager, ObjectListModel<T> *listModel, bool listModelOwned, QWidget *parent=NULL);
		ObjectListWindow (DbManager &manager, QWidget *parent=NULL);
		~ObjectListWindow();

		static void show (DbManager &manager, QWidget *parent=NULL);
		static void show (DbManager &manager, const QString &password, QWidget *parent=NULL);
		static void show (DbManager &manager, bool editPasswordRequired, const QString &editPassword, QWidget *parent=NULL);

		virtual void on_actionNew_triggered ();
		virtual void on_actionEdit_triggered ();
		virtual void on_actionDelete_triggered ();
		virtual void on_actionRefresh_triggered ();

		virtual void on_table_doubleClicked (const QModelIndex &index);

	protected:
		virtual QString makePasswordMessage ();
		void keyPressEvent (QKeyEvent *e);


	private:
		MutableObjectList<T> *list;
		ObjectModel<T> *objectModel;
		ObjectListModel<T> *listModel;
//		bool listModelOwned;

		QSortFilterProxyModel *proxyModel;
};

#endif // OBJECTLISTWINDOW_H
