#ifndef OBJECTLISTWINDOW_H
#define OBJECTLISTWINDOW_H

#include <QtGui/QMainWindow>

#include "ObjectListWindowBase.h"

class DbManager;
class QSortFilterProxyModel;
template<class T> class ObjectListModel;

/**
 * Uses a T::DefaultObjectModel. This model should expose all editable fields
 * of T.
 */
template <class T> class ObjectListWindow: public ObjectListWindowBase
{
	public:
		~ObjectListWindow();

		static void show (DbManager &manager, QWidget *parent=NULL);
		static void show (DbManager &manager, const QString &password, QWidget *parent=NULL);
		static void show (DbManager &manager, bool editPasswordRequired, const QString &editPassword, QWidget *parent=NULL);

		virtual void on_actionNew_triggered ();
		virtual void on_actionEdit_triggered ();
		virtual void on_actionDelete_triggered ();
		virtual void on_actionRefresh_triggered ();

		virtual void on_table_doubleClicked (const QModelIndex &index);
		virtual void on_table_customContextMenuRequested (const QPoint &pos);

		static ObjectListWindow<T> *create (DbManager &manager, QWidget *parent=NULL);

	protected:
		DbManager &manager;


		// Instances are only to be created using the create method
		ObjectListWindow (DbManager &manager, QWidget *parent=NULL);

		int activeObjectCount ();
		QList<T> activeObjects ();

		virtual QString makePasswordMessage ();
		void keyPressEvent (QKeyEvent *e);
		void languageChanged ();

		virtual void prepareContextMenu (QMenu *contextMenu);

	private:
		void appendObjectTo (QList<T> &list, const QModelIndex &tableIndex);
		bool checkAndDelete (const T &object, bool cancelOption);
		void setupText ();

		MutableObjectList<T> *list;
		ObjectModel<T> *objectModel;
		ObjectListModel<T> *listModel;

		QSortFilterProxyModel *proxyModel;

		QMenu *contextMenu;
};

#endif
