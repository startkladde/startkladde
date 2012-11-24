#include "src/algorithms/RectangleLayout.h"

#include <cmath>

RectangleLayout::RectangleLayout ():
	_spacing (0)
{
}

RectangleLayout::~RectangleLayout ()
{
}

void RectangleLayout::setSpacing (int spacing)
{
	_spacing=spacing;
}

// ***********
// ** Items **
// ***********

void RectangleLayout::addItem (int targetY, int h)
{
	Item item;

	item.originalIndex=_items.size ();
	item.y=targetY;
	item.h=h;

	_items.append (item);
}

QList<RectangleLayout::Item> RectangleLayout::items ()
{
	return _items;
}


// ************
// ** Layout **
// ************

// Assumes the list is sorted by target y, uses no weight
double RectangleLayout::overlap (int index) const
{
	// The first item can overlap with the top. All other items can overlap with
	// the previous item (we'll ignore the fact that all items can overlap with
	// the top or with other items than the previous one).

	double minY;
	if (index==0)
		minY=0;
	else
		minY=_items[index-1].y + _items[index-1].h;

	minY+=_spacing;

	double y=_items[index].y;
	if (y<minY)
		return (minY-y);
	else
		return 0;
}

// Assumes the list is sorted by target y, returns the index, uses weight
int RectangleLayout::largestOverlap () const
{
	// Start with the overlap of the first item.
	int maxIndex=-1;
	double maxOverlap=0;

	for (int i=0, n=_items.size (); i<n; ++i)
	{
		double o=overlap (i);

		// Weighting
		if (i>0)
			o*=(sqrt(2)/2);

		if (o>maxOverlap)
		{
			maxIndex=i;
			maxOverlap=o;
		}
	}

	return maxIndex;
}

/** Returns true if done */
bool RectangleLayout::optimizeIteration ()
{
	//std::cout << std::setprecision(3) << "Iteration " << iteration << ", Nodes: ";
	//for (int i=0, n=nodes.size (); i<n; ++i)
	//	std::cout << i << ": " << nodes[i].y << " + " << nodes[i].h << ", ";

	int index=largestOverlap ();

	if (index<0)
		return true;

	double o=overlap (index);

	if (o < 1e-3)
		return true;

	if (index==0)
	{
		_items[index].y+=o;
	}
	else
	{
		_items[index]  .y+=o/2;
		_items[index-1].y-=o/2;
	}

	return false;
}

void RectangleLayout::roundPositions ()
{
	QMutableListIterator<Item> i (_items);
	while (i.hasNext ())
	{
		i.next ();
		Item &item=i.value ();
		item.y=floor (item.y+0.5);
	}
}

void RectangleLayout::forceNoOverlaps ()
{
	int minY=0+_spacing;

	QMutableListIterator<Item> i (_items);
	while (i.hasNext ())
	{
		i.next ();
		Item &item=i.value ();

		if (item.y<minY)
			item.y=minY;

		minY=item.y+item.h+_spacing;
	}
}

void RectangleLayout::doLayout (int maxIterations)
{
	// Sort the list by target y
	qSort (_items);

	// Perform at most maxIterations of optimization
	int iteration;
	for (iteration=0; iteration<maxIterations; ++iteration)
		if (optimizeIteration ())
			break;
	//std::cout << "Optimization done after " << iteration << " iterations" << std::endl;

	// Round the y values
	roundPositions ();

	// Make sure that there is no overlap by moving overlapping widgets down,
	// starting from the top. The solution will not be optimal, but it will be
	// close if we performed enough iterations.
	forceNoOverlaps ();
}
