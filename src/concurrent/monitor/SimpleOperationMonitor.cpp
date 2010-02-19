/*
 * SimpleOperationMonitor.cpp
 *
 *  Created on: Aug 2, 2009
 *      Author: mherrman
 */

#include "SimpleOperationMonitor.h"

#include <iostream>

#include "src/text.h"

SimpleOperationMonitor::SimpleOperationMonitor ()
{
}

SimpleOperationMonitor::~SimpleOperationMonitor ()
{
}

bool SimpleOperationMonitor::isCanceled () const
{
	return false;
}

void SimpleOperationMonitor::status (QString text)
{
	std::cout << "Status: " << text << std::endl;
}

void SimpleOperationMonitor::progress (int progress, int maxProgress)
{
	if (maxProgress>=0)
		std::cout << (QString ("Progress: %1/%2 (%3%)").arg (progress).arg (maxProgress).arg (100*progress/(float)maxProgress)) << std::endl;
	else
		std::cout << "Progress: " << progress << std::endl;
}
