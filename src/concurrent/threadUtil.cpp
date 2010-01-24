/*
 * threadUtil.cpp
 *
 *  Created on: Aug 8, 2009
 *      Author: mherrman
 */

#include "threadUtil.h"

#include <QApplication>
#include <QThread>

QThread *guiThread ()
{
	return qApp->thread ();
}

bool isGuiThread ()
{
	return QThread::currentThread ()==guiThread ();
}
