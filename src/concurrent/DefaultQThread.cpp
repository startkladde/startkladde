/*
 * DefaultQThread.cpp
 *
 *  Created on: Aug 8, 2009
 *      Author: mherrman
 */

#include "DefaultQThread.h"

DefaultQThread::DefaultQThread (QObject *parent):
	QThread (parent)
{
}

DefaultQThread::~DefaultQThread ()
{
}

void DefaultQThread::run ()
{
	exec ();
}
