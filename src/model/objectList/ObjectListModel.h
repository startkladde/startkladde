/*
 * ObjectListModel.h
 *
 *  Created on: Aug 18, 2009
 *      Author: Martin Herrmann
 */

#ifndef OBJECTLISTMODEL_H_
#define OBJECTLISTMODEL_H_

#include <iostream>

#include <QAbstractTableModel>

#include "AbstractObjectList.h"
//#include "ObjectModel.h"

template<class T> class ObjectModel;

/*
 * Potential improvements; this note should go to the proxy model
 *   - add a default sorting column; currently, ObjectListWindow sorts by
 *     column 0 by default.
 *   - allow a custom sorting order (for example for a person: last name,
 *     first name). Note that this requires a way to specify that sort order
 *     rather than "sort by column x".
 */

// *************************
// ** ObjectListModelBase **
// *************************

/**
 * A non-template base class for ObjectListModel. Required for defining slots.
 */
class ObjectListModelBase: public QAbstractTableModel
{
	Q_OBJECT

	public:
		ObjectListModelBase (QObject *parent=NULL): QAbstractTableModel (parent) {}

		void reset () { QAbstractTableModel::reset (); }

	public slots:
		virtual void listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)=0;
};

// *********************
// ** ObjectListModel **
// *********************

/**
 * A table model that gets the rows from an AbstractObjectList and the columns
 * from an ObjectModel.
 *
 * Note that this list cannot be sorted. For a sorted view, use a
 * QSortFilterProxyModel.
 */
template<class T> class ObjectListModel: public ObjectListModelBase
{
	public:
		ObjectListModel (const AbstractObjectList<T> *list, bool listOwned, const ObjectModel<T> *model, bool modelOwned, QObject *parent=NULL);
		virtual ~ObjectListModel ();

		// Access
//		virtual bool hasIndex (const QModelIndex &index) const;
//		virtual bool hasRow (int index) const;
		virtual const T &at (const QModelIndex &index) const;
		virtual const T &at (int row) const;

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

		virtual void listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
		virtual void columnChanged (int column);

		virtual int mapToSource (const QModelIndex &index);
		virtual QModelIndex mapFromSource (int sourceIndex, int column);

	protected:
		const AbstractObjectList<T> *list ; bool  listOwned;
		const ObjectModel       <T> *model; bool modelOwned;
};


// ************************************
// ** ObjectListModel implementation **
// ************************************

/**
 * Creates an ObjectListModel. It can optionally take ownership of the list
 * and the model. If ownership is taken, the corresponding object is deleted
 * when this class is deleted. If ownership is not taken, the objects are not
 * deleted.
 *
 * @param list the object list to get the rows from
 * @param listOwned whether the ObjectListModel takes ownership of the list
 * @param model the model to get the columns from
 * @param modelOwned whether the ObjectListModel takes ownership of the model
 * @param parent the Qt parent of this object
 * @return
 */
template<class T> ObjectListModel<T>::ObjectListModel (const AbstractObjectList<T> *list, bool listOwned, const ObjectModel<T> *model, bool modelOwned, QObject *parent):
	ObjectListModelBase (parent),
	list (list), listOwned (listOwned),
	model (model), modelOwned (modelOwned)
{
#define reemitSignal(signal) do { QObject::connect (list, SIGNAL (signal), this, SIGNAL (signal)); } while (0)
	reemitSignal (columnsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (columnsAboutToBeRemoved (const QModelIndex &, int, int));
	reemitSignal (columnsInserted (const QModelIndex &, int, int));
	reemitSignal (columnsRemoved (const QModelIndex &, int, int));
	reemitSignal (headerDataChanged (Qt::Orientation, int, int));
	reemitSignal (layoutAboutToBeChanged ());
	reemitSignal (layoutChanged ());
	reemitSignal (modelAboutToBeReset ());
	reemitSignal (modelReset ());
	reemitSignal (rowsAboutToBeInserted (const QModelIndex &, int, int));
	reemitSignal (rowsAboutToBeRemoved (const QModelIndex &, int, int));
	reemitSignal (rowsInserted (const QModelIndex &, int, int));
	reemitSignal (rowsRemoved (const QModelIndex &, int, int));
#undef reemitSignal

	// dataChanged is different, see listDataChanged
	QObject::connect (
		list, SIGNAL (dataChanged (const QModelIndex &, const QModelIndex &)),
		this, SLOT (listDataChanged (const QModelIndex &, const QModelIndex &))
	);
}

/**
 * Deletes the model and/or list if they are owned.
 */
template<class T> ObjectListModel<T>::~ObjectListModel ()
{
	if (modelOwned) delete model;
	if (listOwned) delete list;
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

template<class T> int ObjectListModel<T>::rowCount (const QModelIndex &index) const
{
	if (index.isValid ()) return 0;
	return list->size ();
}

template<class T> int ObjectListModel<T>::columnCount (const QModelIndex &index) const
{
	if (index.isValid ()) return 0;
	return model->columnCount ();
}

template<class T> QVariant ObjectListModel<T>::data (const QModelIndex &index, int role) const
{
	int column=index.column ();

	const T &object=list->at (index.row ());
	return model->data (object, column, role);
}

template<class T> QVariant ObjectListModel<T>::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (orientation==Qt::Horizontal)
		return model->headerData (section, role);
	else
		return section+1;
}


// **********
// ** Misc **
// **********

// Untested
//template<class T> bool ObjectListModel<T>::hasRow (int row) const
//{
//	return (row>=0 && row<list->size ());
//}

// Untested
//template<class T> bool ObjectListModel<T>::hasIndex (const QModelIndex &index) const
//{
//	return (
//		index.isValid () &&
//		index.row ()>=0 &&
//		index.column ()>=0 &&
//		index.row ()<list->size () &&
//		index.column ()<model->columnCount ()
//		);
//}

/**
 * Gets a reference to the object specified by a model index from the list
 *
 * @param index the index in this model of the object to be returned; only the
 *              row of the model index is used
 * @return a reference to the object
 */
template<class T> const T &ObjectListModel<T>::at (const QModelIndex &index) const
{
	return list->at (index.row ());
}

/**
 * Gets a reference to the object specified by a row number
 *
 * @param index the row of the object to be returned
 * @return a reference to the object
 */
template<class T> const T &ObjectListModel<T>::at (int row) const
{
	return list->at (row);
}

/**
 * Will be called when the contents of the list change. All other change
 * signals of AbstractObjectList refer to rows as such and can thus be
 * reemitted without change. However, the dataChanged signal refers to colums,
 * of which the AbstractObjectList only has one, but the ObjectListModel can
 * have several (as determined by the object model). Thus, we have to emit
 * the signal with different parameters.
 *
 * @param topLeft the upper left index of the data that changed; the index
 *                refers to the AbstractObjectList (which has only one column)
 * @param bottomRight the bottom right index of the data that changed; the index
 *                refers to the AbstractObjectList (which has only one column)
 */
template<class T> void ObjectListModel<T>::listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	// As this is only called after the data changed, we cannot determine which
	// of the columns are affected, so we emit a change of all columns
	QModelIndex newTopLeft=createIndex (topLeft.row (), 0);
	QModelIndex newBottomRight=createIndex (bottomRight.row (), model->columnCount ()-1);

	emit dataChanged (newTopLeft, newBottomRight);
}

/**
 * Emits a change of all values of the specified column. This may be useful to
 * refresh a column whose value depends on the time.
 *
 * @param column the number of the column; must be >=0 and <columnCount
 */
template<class T> void ObjectListModel<T>::columnChanged (int column)
{
	QModelIndex topLeft=createIndex (0, column);
	QModelIndex bottomRight=createIndex (list->size (), column);

	emit dataChanged (topLeft, bottomRight);
}

// FIXME document
template<class T> int ObjectListModel<T>::mapToSource (const QModelIndex &index)
{
	if (!index.isValid ())
		return -1;

	// That's easy, because the objectListModel's rows correspond to the source
	// model's entries 1:1.
	return index.row ();
}

// FIXME document
template<class T> QModelIndex ObjectListModel<T>::mapFromSource (int sourceIndex, int column)
{
	if (sourceIndex<0)
		return QModelIndex ();

	// That's easy, because the objectListModel's rows correspond to the source
	// model's entries 1:1.
	return this->index (sourceIndex, column);
}

#endif
