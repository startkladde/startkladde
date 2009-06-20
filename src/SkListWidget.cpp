#include "SkListWidget.h"

SkListWidget::SkListWidget (QWidget *parent)/*{{{*/
	:QListWidget (parent)
{
}/*}}}*/

QSize SkListWidget::sizeHint () const/*{{{*/
{
	QSize sh=QListWidget::sizeHint ();
	sh.setHeight (60);
	return sh;
}
/*}}}*/

