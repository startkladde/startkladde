//#ifndef CACHINGOBJECTLISTMODEL_H_
//#define CACHINGOBJECTLISTMODEL_H_
//
////#include <iostream>
//
//#include "ObjectListModel.h"
//
//
///**
// * A subclass of ObjectListModel that caches the data produced by the model
// *
// * The header data is not cached.
// */
//template<class T> class CachingObjectListModel: public ObjectListModel<T>
//{
//	public:
//		CachingObjectListModel (const AbstractObjectList<T> *list, bool listOwned, const ObjectModel<T> *model, bool modelOwned, QObject *parent=NULL);
//		virtual ~CachingObjectListModel ();
//
////		virtual QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
////		virtual void listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight);
////		virtual void columnChanged (int column);
//
//	protected:
//};
//
//
//// *******************************************
//// ** CachingObjectListModel implementation **
//// *******************************************
//
///**
// * See ObjectListModel<T>::ObjectListModel
// */
//template<class T> CachingObjectListModel<T>::CachingObjectListModel (const AbstractObjectList<T> *list, bool listOwned, const ObjectModel<T> *model, bool modelOwned, QObject *parent):
//	ObjectListModel (list, listOwned, model, modelOwned, parent)
//{
//	// Implement: receive more signals, maintain cache
//}
//
//template<class T> CachingObjectListModel<T>::~CachingObjectListModel ()
//{
//}
//
//
////template<class T> QVariant ObjectListModel<T>::data (const QModelIndex &index, int role) const
////{
////	// Implement
////}
////
////template<class T> void ObjectListModel<T>::listDataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
////{
////	// Implement
////}
////
////template<class T> void ObjectListModel<T>::columnChanged (int column)
////{
////	// Implement
////}
//
//
//#endif
