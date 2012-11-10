#ifndef FLIGHTTABLEVIEW_H_
#define FLIGHTTABLEVIEW_H_

#include "SkTableView.h"

#include <QPersistentModelIndex>

#include "src/db/dbId.h"
#include "src/db/DbManager.h"
#include "src/FlightReference.h"

class QSettings;

template<class T> class EntityList;
class FlightProxyList;
class FlightModel;
template<class T> class ObjectListModel;
class FlightSortFilterProxyModel;


class FlightTableView: public SkTableView
{
		Q_OBJECT

	public:
		FlightTableView (QWidget *parent);
		virtual ~FlightTableView ();

		void setModel (EntityList<Flight> *flightList, DbManager &dbManager);

		FlightReference getCurrentFlightReference ();
		bool selectFlight (const FlightReference &flight, int column);

		void readColumnWidths (QSettings &settings);
		void writeColumnWidths (QSettings &settings);

		QRectF rectForFlight (const FlightReference &flight, int column) const;


	public slots:
		void setHideFinishedFlights (bool hideFinishedFlights);
		void setAlwaysShowExternalFlights (bool alwaysShowExternalFlights);
		void setAlwaysShowErroneousFlights (bool alwaysShowErroneousFlights);
		void setShowPreparedFlights (bool showPreparedFlights);
		void setCustomSorting (bool customSorting);

		void sortCustom ();
		void sortByColumn (int column);

		void interactiveJumpToTowflight ();

		void languageChanged ();
		void minuteChanged ();

		void showNotification (const FlightReference &flight, const QString &message, int milliseconds);


	signals:
		void departButtonClicked (FlightReference flight);
		void landButtonClicked (FlightReference flight);

	protected slots:
		void base_buttonClicked (QPersistentModelIndex proxyIndex);

	private:
		// The models involved in displaying the flight list
		EntityList<Flight> *_flightList;
		FlightProxyList *_proxyList;
		FlightModel *_flightModel;
		ObjectListModel<Flight> *_flightListModel;
		FlightSortFilterProxyModel *_proxyModel;

		Qt::SortOrder _sortOrder;
		int _sortColumn;

};

#endif
