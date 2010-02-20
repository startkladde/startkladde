/*
 * FlightSortFilterProxyModel.h
 *
 *  Created on: Sep 3, 2009
 *      Author: Martin Herrmann
 */

#ifndef _FlightSortFilterProxyModel_h
#define _FlightSortFilterProxyModel_h

#include <QSortFilterProxyModel>

class DataStorage;

class FlightSortFilterProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		FlightSortFilterProxyModel (DataStorage &dataStorage, QObject *parent);
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
		DataStorage &dataStorage;

		// Filter options
		bool showPreparedFlights; // TODO: remove this, the main window takes care of that
		bool hideFinishedFlights;
		bool alwaysShowExternalFlights;
		bool alwaysShowErroneousFlights;

		// Sort options
		bool customSorting;
};

#endif /* FLIGHTSORTFILTERPROXYMODEL_H_ */
