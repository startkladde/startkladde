/*
 * FlightSortFilterProxyModel.h
 *
 *  Created on: Sep 3, 2009
 *      Author: Martin Herrmann
 */

#ifndef FLIGHTSORTFILTERPROXYMODEL_H_
#define FLIGHTSORTFILTERPROXYMODEL_H_

#include <QSortFilterProxyModel>

namespace Db { namespace Cache { class Cache; } }

class FlightSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		FlightSortFilterProxyModel (Db::Cache::Cache &cache, QObject *parent);
		virtual ~FlightSortFilterProxyModel ();

	public slots:
		virtual void setShowPreparedFlights        (bool showPreparedFlights       ) { this->showPreparedFlights       =showPreparedFlights       ; invalidate (); }
		virtual void setHideFinishedFlights        (bool hideFinishedFlights       ) { this->hideFinishedFlights       =hideFinishedFlights       ; invalidate (); }
		virtual void setAlwaysShowExternalFlights  (bool alwaysShowExternalFlights ) { this->alwaysShowExternalFlights =alwaysShowExternalFlights ; invalidate (); }
		virtual void setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights) { this->alwaysShowErroneousFlights=alwaysShowErroneousFlights; invalidate (); }
		virtual void setCustomSorting (bool customSorting);
		virtual bool getCustomSorting () const { return customSorting; }
		virtual void sortCustom ();

	protected:
		virtual bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const;
		virtual bool lessThan (const QModelIndex &left, const QModelIndex &right) const;

	private:
		Db::Cache::Cache &cache;

		// Filter options
		bool showPreparedFlights; // TODO: remove this, the main window takes care of that
		bool hideFinishedFlights;
		bool alwaysShowExternalFlights;
		bool alwaysShowErroneousFlights;

		// Sort options
		bool customSorting;
};

#endif
