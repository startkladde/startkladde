#ifndef FLARMNETOVERVIEW_H
#define FLARMNETOVERVIEW_H

#include "ui_FlarmNetOverview.h"

#include "src/gui/SkDialog.h"

class DbManager;

class FlarmNetOverview: public SkDialog<Ui::FlarmNetOverviewDialog>
{
	Q_OBJECT

	public:
		FlarmNetOverview (DbManager &dbManager, QWidget *parent);
		~FlarmNetOverview ();
};

#endif
