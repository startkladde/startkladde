#ifndef FLARMNETOVERVIEW_H
#define FLARMNETOVERVIEW_H

#include "ui_FlarmNetOverview.h"

#include "src/db/DbManager.h"
#include "src/gui/SkDialog.h"

class FlarmNetOverview: public SkDialog<Ui::FlarmNetOverviewDialog>
{
	Q_OBJECT

	public:
		FlarmNetOverview (QWidget *parent, DbManager* db);
		~FlarmNetOverview ();

	private:
		DbManager* dbManager;
};

#endif
