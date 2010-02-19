/*
 * SleepTask.cpp
 *
 *  Created on: Aug 9, 2009
 *      Author: mherrman
 */

#include "SleepTask.h"

#include <iostream>

#include <QTime>
#include <QThread>

#include "src/concurrent/threadUtil.h"
#include "src/concurrent/DefaultQThread.h"

SleepTask::SleepTask (int seconds):
	seconds (seconds)
{
}

SleepTask::~SleepTask ()
{
}

bool SleepTask::run ()
{
	std::cout << "SleepTask " << this << " waiting for " << seconds << " seconds on " << (isGuiThread ()?"the GUI thread":"a background thread") << std::endl;
	int i;

	for (i=0; i<seconds && !isCanceled (); ++i)
	{
		emit progress (i, seconds);
		DefaultQThread::msleep (1000);
	}

	return (i==seconds);
}

QString SleepTask::toString () const
{
	return QString ("Sleep for %1 seconds").arg (seconds);
}
