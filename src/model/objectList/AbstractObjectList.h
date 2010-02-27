/*
 * AbstractObjectList.h
 *
 *  Created on: Sep 1, 2009
 *      Author: Martin Herrmann
 */

#ifndef ABSTRACTOBJECTLIST_H_
#define ABSTRACTOBJECTLIST_H_

#include <QAbstractTableModel>

/**
 * A list of objects that performs as a QAbstractTableModel, thus emitting
 * signals on change.
 *
 * The data provided by this is a table with a single column, containing the
 * result of the object's toString method. For a custom table model, use
 * ObjectListModel.
 *
 * Note that this QObject can be a template as it defines no signals or slots.
 */
template<class T> class AbstractObjectList: public QAbstractTableModel
{
	public:
		// Construction
		AbstractObjectList (QObject *parent=NULL);
		virtual ~AbstractObjectList ();

		// Access
		using QObject::parent; // Hidden by QAbstractItemModel::parent (const QModelIndex &)

		/**
		 * The number of objects in the list
		 *
		 * @return the number of objects in the list; always >=0
		 */
		virtual int size () const=0;

		/**
		 * Gets a reference to the object a a given position in the list
		 *
		 * @param index the list index; must be >=0 and <size()
		 * @return a reference to the object a the specified index
		 */
		virtual const T &at (int index) const=0;

		/**
		 * Returns a list of objects in this ObjectList. This is probably slow
		 * (as all objects have to be copied) unless the ObjectList
		 * implementation is based on a QList, implicit sharing is used, and
		 * neither the returned list nor the ObjectList is modified.
		 *
		 * @return a QList consisting of the individual objects
		 */
		virtual QList<T> getList () const=0;

		// QAbstractTableModel methods
		virtual int rowCount (const QModelIndex &index) const;
		virtual int columnCount (const QModelIndex &index) const;
		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
		virtual QVariant headerData (int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
};

template<class T> AbstractObjectList<T>::AbstractObjectList (QObject *parent):
	QAbstractTableModel (parent)
{
}

template<class T> AbstractObjectList<T>::~AbstractObjectList ()
{
}


// *********************************
// ** QAbstractTableModel methods **
// *********************************

/**
 * The number of rows in the model - this is the number of objects in the list
 * for the root index, invalid else.
 *
 * @see QAbstractTableModel::rowCount
 */
template<class T> int AbstractObjectList<T>::rowCount (const QModelIndex &index) const
{
	if (index.isValid ()) return 0;
	return size ();
}

/**
 * The number of columns in the model - 1 for the root index, invalid else
 *
 * @see QAbstractTableModel::columnCount
 */
template<class T> int AbstractObjectList<T>::columnCount (const QModelIndex &index) const
{
	if (index.isValid ()) return 0;
	return 1;
}

/**
 * The data for a given model index and role. Uses the toString method of the
 * object at the given index for the DisplayRole
 *
 * @see QAbstractTableModel::data
 */
template<class T> QVariant AbstractObjectList<T>::data (const QModelIndex &index, int role) const
{
	if (role!=Qt::DisplayRole) return QVariant ();

	const T &object=at (index.row ());
	return object.toString ();
}

/**
 * The header data for a given orientation and role. Returns the section
 * (starting at 1) for the display role of the vertical header, and invalid
 * else.
 *
 * @see QAbstractTableModel::headerDatas
 */
template<class T> QVariant AbstractObjectList<T>::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role!=Qt::DisplayRole) return QVariant ();

	if (orientation==Qt::Horizontal)
		return QVariant ();
	else
		return section+1;
}


#endif
