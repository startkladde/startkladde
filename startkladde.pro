CONFIG += qt
CONFIG += debug
QT += qt3support
QT += sql
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
#LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi
LIBS += -lz -lacpi
MAKEFILE = Makefile_startkladde
OBJECTS_DIR = build/
MOC_DIR= build/
#QMAKE_CXXFLAGS += -Werror

# Input
HEADERS += \
           build/migrations.h \
           build/migrations_headers.h \
           src/SkException.h \
           src/accessor.h \
           src/color.h \
           src/text.h \
           src/version.h \
           src/concurrent/DefaultQThread.h \
           src/concurrent/ThreadUtil.h \
           src/concurrent/WorkerThread.h \
           src/concurrent/monitor/OperationMonitor.h \
           src/concurrent/monitor/SimpleOperationMonitor.h \
           src/concurrent/task/SleepTask.h \
           src/concurrent/task/Task.h \
           src/config/Options.h \
           src/db/AbstractDatabase.h \
           src/db/Database.h \
           src/db/DatabaseTask.h \
           src/db/DatabaseThread.h \
           src/db/DatabaseWorker.h \
           src/db/DbEvent.h \
           src/db/ThreadSafeDatabase.h \
           src/db/dbId.h \
           src/db/dataStorage/DataStorage.h \
           src/db/dataStorage/DataStorageMonitor.h \
           src/db/dataStorage/DataStorageWorker.h \
           src/db/interface/DatabaseInterface.h \
           src/db/migration/Migration.h \
           src/db/migration/MigrationFactory.h \
           src/db/migration/Migrator.h \
           src/db/schema/CurrentSchema.h \
           src/db/schema/Schema.h \
           src/db/schema/SchemaDumper.h \
           src/db/task/AddObjectTask.h \
           src/db/task/DataStorageSleepTask.h \
           src/db/task/DeleteObjectTask.h \
           src/db/task/FetchFlightsTask.h \
           src/db/task/ObjectUsedTask.h \
           src/db/task/RefreshAllTask.h \
           src/db/task/UpdateObjectTask.h \
           src/gui/dialogs.h \
           src/gui/widgets/AcpiWidget.h \
           src/gui/widgets/SkComboBox.h \
           src/gui/widgets/SkLabel.h \
           src/gui/widgets/SkTableView.h \
           src/gui/widgets/SkTreeWidgetItem.h \
           src/gui/widgets/TableButton.h \
           src/gui/widgets/WeatherWidget.h \
           src/gui/windows/DateInputDialog.h \
           src/gui/windows/EntitySelectWindow.h \
           src/gui/windows/FlightWindow.h \
           src/gui/windows/MainWindow.h \
           src/gui/windows/ObjectListWindow.h \
           src/gui/windows/ObjectListWindowBase.h \
           src/gui/windows/SplashScreen.h \
           src/gui/windows/StatisticsWindow.h \
           src/gui/windows/TaskProgressDialog.h \
           src/gui/windows/WeatherDialog.h \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorWindow.h \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.h \
           src/gui/windows/objectEditor/PersonEditorPane.h \
           src/gui/windows/objectEditor/PlaneEditorPane.h \
           src/io/TempDir.h \
           src/io/colors.h \
           src/io/io.h \
           src/logging/messages.h \
           src/model/Entity.h \
           src/model/Flight.h \
           src/model/LaunchMethod.h \
           src/model/Person.h \
           src/model/Plane.h \
           src/model/flightList/FlightModel.h \
           src/model/flightList/FlightProxyList.h \
           src/model/flightList/FlightSortFilterProxyModel.h \
           src/model/objectList/AbstractObjectList.h \
           src/model/objectList/AutomaticEntityList.h \
           src/model/objectList/ColumnInfo.h \
           src/model/objectList/EntityList.h \
           src/model/objectList/MutableObjectList.h \
           src/model/objectList/ObjectListModel.h \
           src/model/objectList/ObjectModel.h \
           src/plugins/ShellPlugin.h \
           src/statistics/LaunchMethodStatistics.h \
           src/statistics/PilotLog.h \
           src/statistics/PlaneLog.h \
           src/time/Date.h \
           src/time/Time.h \
           src/time/timeFunctions.h \
           src/util/bool.h \
           src/util/qString.h \
		   # Empty line

SOURCES += \
		   build/CurrentSchema.cpp \
           src/color.cpp \
           src/startkladde.cpp \
           src/text.cpp \
           src/version.cpp \
           src/concurrent/DefaultQThread.cpp \
           src/concurrent/ThreadUtil.cpp \
           src/concurrent/WorkerThread.cpp \
           src/concurrent/monitor/OperationMonitor.cpp \
           src/concurrent/monitor/SimpleOperationMonitor.cpp \
           src/concurrent/task/SleepTask.cpp \
           src/concurrent/task/Task.cpp \
           src/config/Options.cpp \
           src/db/AbstractDatabase.cpp \
           src/db/Database.cpp \
           src/db/DatabaseTask.cpp \
           src/db/DatabaseThread.cpp \
           src/db/DatabaseWorker.cpp \
           src/db/DbEvent.cpp \
           src/db/ThreadSafeDatabase.cpp \
           src/db/dbId.cpp \
           src/db/dataStorage/DataStorage.cpp \
           src/db/dataStorage/DataStorageMonitor.cpp \
           src/db/dataStorage/DataStorageWorker.cpp \
           src/db/interface/DatabaseInterface.cpp \
           src/db/migration/Migration.cpp \
           src/db/migration/MigrationFactory.cpp \
           src/db/migration/Migrator.cpp \
           src/db/schema/Schema.cpp \
           src/db/schema/SchemaDumper.cpp \
           src/db/task/DataStorageSleepTask.cpp \
           src/db/task/FetchFlightsTask.cpp \
           src/db/task/RefreshAllTask.cpp \
           src/gui/dialogs.cpp \
           src/gui/widgets/AcpiWidget.cpp \
           src/gui/widgets/SkComboBox.cpp \
           src/gui/widgets/SkLabel.cpp \
           src/gui/widgets/SkTableView.cpp \
           src/gui/widgets/SkTreeWidgetItem.cpp \
           src/gui/widgets/TableButton.cpp \
           src/gui/widgets/WeatherWidget.cpp \
           src/gui/windows/DateInputDialog.cpp \
           src/gui/windows/FlightWindow.cpp \
           src/gui/windows/MainWindow.cpp \
           src/gui/windows/ObjectListWindow.cpp \
           src/gui/windows/ObjectListWindowBase.cpp \
           src/gui/windows/SplashScreen.cpp \
           src/gui/windows/StatisticsWindow.cpp \
           src/gui/windows/TaskProgressDialog.cpp \
           src/gui/windows/WeatherDialog.cpp \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.cpp \
           src/gui/windows/objectEditor/PersonEditorPane.cpp \
           src/gui/windows/objectEditor/PlaneEditorPane.cpp \
           src/io/TempDir.cpp \
           src/io/io.cpp \
           src/logging/messages.cpp \
           src/model/Entity.cpp \
           src/model/Flight.cpp \
           src/model/Flight_Mode.cpp \
           src/model/Flight_Type.cpp \
           src/model/LaunchMethod.cpp \
           src/model/Person.cpp \
           src/model/Plane.cpp \
           src/model/flightList/FlightModel.cpp \
           src/model/flightList/FlightProxyList.cpp \
           src/model/flightList/FlightSortFilterProxyModel.cpp \
           src/plugins/ShellPlugin.cpp \
           src/statistics/LaunchMethodStatistics.cpp \
           src/statistics/PilotLog.cpp \
           src/statistics/PlaneLog.cpp \
           src/time/Date.cpp \
           src/time/Time.cpp \
           src/time/timeFunctions.cpp \
           src/util/bool.cpp \
           src/util/qString.cpp \
		   # Empty line

FORMS += \
           src/gui/windows/DateInputDialog.ui \
           src/gui/windows/FlightWindow.ui \
           src/gui/windows/MainWindow.ui \
           src/gui/windows/ObjectListWindowBase.ui \
           src/gui/windows/StatisticsWindow.ui \
           src/gui/windows/TaskProgressDialog.ui \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.ui \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.ui \
           src/gui/windows/objectEditor/PersonEditorPane.ui \
           src/gui/windows/objectEditor/PlaneEditorPane.ui \
           # Empty line

!include( build/migrations.pro ) {
error( "build/migrations.pro could not be included" )
}

