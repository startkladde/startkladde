#ifndef QLIST_H_
#define QLIST_H_

#include <QList>

template<class T> void deleteList (QList<T *> &list)
{
	while (!list.isEmpty ())
		delete list.takeLast ();
}

#endif
