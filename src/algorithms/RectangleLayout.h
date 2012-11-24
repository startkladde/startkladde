#ifndef RECTANGLELAYOUT_H_
#define RECTANGLELAYOUT_H_

#include <QList>

class RectangleLayout
{
	public:
		class Item
		{
			public:
				int originalIndex;
				double y;
				int h;
				bool operator< (const Item &other) const { return y<other.y; }
		};

		RectangleLayout ();
		virtual ~RectangleLayout ();

		void setSpacing (int spacing);

		void addItem (int targetY, int h);
		QList<Item> items ();

		void doLayout (int maxIterations);


	private:
		double overlap (int index) const;
		int largestOverlap () const;

		bool optimizeIteration ();
		void roundPositions ();
		void forceNoOverlaps ();


		int _spacing;
		QList<Item> _items;

};

#endif
