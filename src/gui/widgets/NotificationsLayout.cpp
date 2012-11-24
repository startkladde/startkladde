#include "src/gui/widgets/NotificationsLayout.h"

#include <iostream>
#include <cmath>
#include <iomanip>

#include "src/util/math.h"
#include "src/algorithms/RectangleLayout.h"

NotificationsLayout::NotificationsLayout ()
{
}

NotificationsLayout::~NotificationsLayout ()
{
}

void NotificationsLayout::add (NotificationWidget *widget)
{
	Node node;

	node.widget=widget;
	node.visible=false;

	_nodes.insert (widget, node);
}

void NotificationsLayout::remove (const NotificationWidget *widget)
{
	_nodes.remove (widget);
}

void NotificationsLayout::setArrowPosition (const NotificationWidget *widget, const QPoint &position)
{
	Node &node=_nodes[widget];

	node.arrowPosition=position;
	node.visible=true;
}

void NotificationsLayout::setWidgetInvisible (const NotificationWidget *widget)
{
	Node &node=_nodes[widget];

	node.visible=false;
}


void NotificationsLayout::doLayout ()
{
	QList<Node> visibleNodes;

	// Iterate over all nodes. Make a list of visible nodes and hide all
	// invisible nodes.
	QList<Node> nodes=_nodes.values ();
	for (int i=0, n=nodes.size (); i<n; ++i)
	{
		Node &node=nodes[i];

		if (node.visible)
			visibleNodes.append (node);
		else
			node.widget->hide ();
	}

	// Add the visible nodes to a rectangle layout
	RectangleLayout layout;
	layout.setSpacing (1);
	for (int i=0, n=visibleNodes.size (); i<n; ++i)
	{
		const Node &node=visibleNodes[i];

		QPoint arrowPosition=node.arrowPosition;
		NotificationWidget *widget=node.widget;

		int targetY=widget->defaultBubblePosition (arrowPosition).y ();
		int h=widget->bubbleGeometryParent ().height ();

		layout.addItem (targetY, h);
	}

	// Perform the layout
	layout.doLayout (50);

	// Apply the layout
	QList<RectangleLayout::Item> layoutItems=layout.items ();
	for (int i=0, n=layoutItems.size (); i<n; ++i)
	{
		RectangleLayout::Item &item=layoutItems[i];

		Node &node=visibleNodes[item.originalIndex];

		QPoint arrowPosition=node.arrowPosition;
		QPoint bubblePosition=node.widget->defaultBubblePosition (arrowPosition);

		bubblePosition.setY (item.y);

		node.widget->moveTo (arrowPosition, bubblePosition);
		node.widget->show ();
	}
}
