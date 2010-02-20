/*
 * TableButton.h
 *
 *  Created on: Aug 30, 2009
 *      Author: Martin Herrmann
 */

#ifndef TABLEBUTTON_H_
#define TABLEBUTTON_H_

#include <QPushButton>
#include <QPersistentModelIndex>

/**
 * A QPersistentModelIndex is used, which will be updated to be identical to
 * the new index as the model changes.
 */
class TableButton: public QPushButton
{
	Q_OBJECT

	public:
		TableButton (QPersistentModelIndex index, QWidget *parent=0);
		TableButton (QPersistentModelIndex index, const QString &text, QWidget *parent=0);
		TableButton (QPersistentModelIndex index, const QIcon &icon, const QString &text, QWidget *parent=0);

		virtual ~TableButton ();

		QPersistentModelIndex getIndex () { return index; }

		virtual QSize sizeHint () const;

	signals:
		void clicked (QPersistentModelIndex index);

	protected slots:
		void clickedSlot ();

	private:
		void init ();

		QPersistentModelIndex index;
};

#endif /* TABLEBUTTON_H_ */
