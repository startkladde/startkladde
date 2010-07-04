##############
## Settings ##
##############

win32: MYSQL_INCLUDE_PATH = c:/programme/mysql/5.1/include


#########################
## Build configuration ##
#########################

TEMPLATE = app
CONFIG += qt thread resources
QT += network sql

# Do not overwrite the to level Makefile
MAKEFILE = Makefile_startkladde

# On Windows, build an application without console
win32:CONFIG += windows

# Build in both debug and release mode and set the SK_BUILD variable
CONFIG -= debug_and_release_target
CONFIG += debug_and_release
CONFIG(debug, debug|release) {
	SK_BUILD=debug
} else {
	SK_BUILD=release
}

# qmake stinkage: we cannot install the target with a different name from the
# one it was created with. Specifically, we would like to install
# startkladde_release as /usr/bin/startkladde. Using target.extra to make a
# copy of the file before installing fails as the install command is not
# performed.
# This is what we would like to do:
# TARGET = startkladde_$${SK_BUILD}
CONFIG(debug, debug|release) {
	TARGET = startkladde_debug
} else {
	TARGET = startkladde
}

DEFINES += SK_BUILD=$${SK_BUILD}

#QMAKE_CXXFLAGS += -Werror


###########
## Paths ##
###########

MOC_DIR= build
UI_DIR= build
RCC_DIR= build

#DEPENDPATH += . version
INCLUDEPATH += .

unix {
	INCLUDEPATH += /usr/include/mysql
	# Link against mysqlclient explicitly to avoid "Error in my_thread_global_end():
	# 1 threads didn't exit" in specific situations (e. g. exception on open).
	LIBS += -lmysqlclient
}
win32 {
	INCLUDEPATH += $$MYSQL_INCLUDE_PATH
}



#################
## Input files ##
#################

HEADERS += \
           build/migrations.h \
           build/migrations_headers.h \
           src/accessor.h \
           src/color.h \
           src/StorableException.h \
           src/text.h \
           src/version.h \
           src/concurrent/DefaultQThread.h \
           src/concurrent/Returner.h \
           src/concurrent/synchronized.h \
           src/concurrent/ThreadUtil.h \
           src/concurrent/Waiter.h \
           src/concurrent/monitor/OperationCanceledException.h \
           src/concurrent/monitor/OperationMonitor.h \
           src/concurrent/monitor/OperationMonitorInterface.h \
           src/concurrent/monitor/SignalOperationMonitor.h \
           src/concurrent/monitor/SimpleOperationMonitor.h \
           src/config/Settings.h \
           src/container/SortedSet.h \
           src/container/SortedSet_impl.h \
           src/db/Database.h \
           src/db/DbManager.h \
           src/db/DbWorker.h \
           src/db/Query.h \
           src/db/dbId.h \
           src/db/cache/Cache.h \
           src/db/cache/CacheWorker.h \
           src/db/event/DbEvent.h \
           src/db/event/DbEventMonitor.h \
           src/db/interface/AbstractInterface.h \
           src/db/interface/DefaultInterface.h \
           src/db/interface/Interface.h \
           src/db/interface/InterfaceWorker.h \
           src/db/interface/ThreadSafeInterface.h \
           src/db/interface/exceptions/ConnectionFailedException.h \
           src/db/interface/exceptions/PingFailedException.h \
           src/db/interface/exceptions/QueryFailedException.h \
           src/db/interface/exceptions/TransactionFailedException.h \
           src/db/interface/exceptions/SqlException.h \
           src/db/interface/exceptions/AccessDeniedException.h \
           src/db/interface/exceptions/DatabaseDoesNotExistException.h \
           src/db/migration/Migration.h \
           src/db/migration/MigrationFactory.h \
           src/db/migration/Migrator.h \
           src/db/migration/MigratorWorker.h \
           src/db/result/CopiedResult.h \
           src/db/result/DefaultResult.h \
           src/db/result/Result.h \
           src/db/schema/CurrentSchema.h \
           src/db/schema/Schema.h \
           src/db/schema/SchemaDumper.h \
           src/db/schema/spec/ColumnSpec.h \
           src/db/schema/spec/IndexSpec.h \
           src/gui/dialogs.h \
           src/gui/views/ReadOnlyItemDelegate.h \
           src/gui/views/SpecialIntDelegate.h \
           src/gui/views/SpinBoxCreator.h \
           src/gui/widgets/AcpiWidget.h \
           src/gui/widgets/SkComboBox.h \
           src/gui/widgets/SkLabel.h \
           src/gui/widgets/SkTableView.h \
           src/gui/widgets/SkTreeWidgetItem.h \
           src/gui/widgets/TableButton.h \
           src/gui/widgets/WeatherWidget.h \
           src/gui/windows/DateInputDialog.h \
           src/gui/windows/FlightWindow.h \
           src/gui/windows/LaunchMethodSelectionWindow.h \
           src/gui/windows/MainWindow.h \
           src/gui/windows/MonitorDialog.h \
           src/gui/windows/ObjectListWindow.h \
           src/gui/windows/ObjectListWindowBase.h \
           src/gui/windows/ObjectSelectWindow.h \
           src/gui/windows/ObjectSelectWindowBase.h \
           src/gui/windows/SplashScreen.h \
           src/gui/windows/SettingsWindow.h \
           src/gui/windows/StatisticsWindow.h \
           src/gui/windows/WeatherDialog.h \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorWindow.h \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.h \
           src/gui/windows/objectEditor/PersonEditorPane.h \
           src/gui/windows/objectEditor/PlaneEditorPane.h \
           src/io/AnsiColors.h \
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
           src/net/TcpProxy.h \
           src/plugin/Plugin.h \
           src/plugin/ShellPlugin.h \
           src/plugin/ShellPluginInfo.h \
           src/plugin/info/InfoPlugin.h \
           src/plugin/info/InfoPluginFactory.h \
           src/plugins/info/TestPlugin.h \
           src/statistics/LaunchMethodStatistics.h \
           src/statistics/PilotLog.h \
           src/statistics/PlaneLog.h \
           src/util/bool.h \
           src/util/qList.h \
           src/util/qString.h \
           src/util/time.h \
		   # Empty line

SOURCES += \
		   build/CurrentSchema.cpp \
           src/color.cpp \
           src/startkladde.cpp \
           src/StorableException.cpp \
           src/text.cpp \
           src/version.cpp \
           src/concurrent/DefaultQThread.cpp \
           src/concurrent/Returner.cpp \
           src/concurrent/synchronized.cpp \
           src/concurrent/ThreadUtil.cpp \
           src/concurrent/Waiter.cpp \
           src/concurrent/monitor/OperationCanceledException.cpp \
           src/concurrent/monitor/OperationMonitor.cpp \
           src/concurrent/monitor/OperationMonitorInterface.cpp \
           src/concurrent/monitor/SignalOperationMonitor.cpp \
           src/concurrent/monitor/SimpleOperationMonitor.cpp \
           src/config/Settings.cpp \
           src/db/Database.cpp \
           src/db/DatabaseInfo.cpp \
           src/db/DbManager.cpp \
           src/db/DbWorker.cpp \
           src/db/Query.cpp \
           src/db/dbId.cpp \
           src/db/cache/Cache.cpp \
           src/db/cache/Cache_hashUpdates.cpp \
           src/db/cache/Cache_lookup.cpp \
           src/db/cache/CacheWorker.cpp \
           src/db/event/DbEvent.cpp \
           src/db/event/DbEventMonitor.cpp \
           src/db/interface/AbstractInterface.cpp \
           src/db/interface/DefaultInterface.cpp \
           src/db/interface/Interface.cpp \
           src/db/interface/InterfaceWorker.cpp \
           src/db/interface/ThreadSafeInterface.cpp \
           src/db/interface/exceptions/ConnectionFailedException.cpp \
           src/db/interface/exceptions/PingFailedException.cpp \
           src/db/interface/exceptions/QueryFailedException.cpp \
           src/db/interface/exceptions/TransactionFailedException.cpp \
           src/db/interface/exceptions/SqlException.cpp \
           src/db/interface/exceptions/AccessDeniedException.cpp \
           src/db/interface/exceptions/DatabaseDoesNotExistException.cpp \
           src/db/migration/Migration.cpp \
           src/db/migration/MigrationFactory.cpp \
           src/db/migration/Migrator.cpp \
           src/db/migration/MigratorWorker.cpp \
           src/db/result/CopiedResult.cpp \
           src/db/result/DefaultResult.cpp \
           src/db/schema/Schema.cpp \
           src/db/schema/SchemaDumper.cpp \
           src/db/schema/spec/ColumnSpec.cpp \
           src/db/schema/spec/IndexSpec.cpp \
           src/gui/dialogs.cpp \
           src/gui/views/ReadOnlyItemDelegate.cpp \
           src/gui/views/SpecialIntDelegate.cpp \
           src/gui/views/SpinBoxCreator.cpp \
           src/gui/widgets/SkComboBox.cpp \
           src/gui/widgets/SkLabel.cpp \
           src/gui/widgets/SkTableView.cpp \
           src/gui/widgets/SkTreeWidgetItem.cpp \
           src/gui/widgets/TableButton.cpp \
           src/gui/widgets/WeatherWidget.cpp \
           src/gui/windows/DateInputDialog.cpp \
           src/gui/windows/FlightWindow.cpp \
           src/gui/windows/LaunchMethodSelectionWindow.cpp \
           src/gui/windows/MainWindow.cpp \
           src/gui/windows/MonitorDialog.cpp \
           src/gui/windows/ObjectListWindow.cpp \
           src/gui/windows/ObjectListWindowBase.cpp \
           src/gui/windows/ObjectSelectWindow.cpp \
           src/gui/windows/ObjectSelectWindowBase.cpp \
           src/gui/windows/SplashScreen.cpp \
           src/gui/windows/SettingsWindow.cpp \
           src/gui/windows/StatisticsWindow.cpp \
           src/gui/windows/WeatherDialog.cpp \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.cpp \
           src/gui/windows/objectEditor/PersonEditorPane.cpp \
           src/gui/windows/objectEditor/PlaneEditorPane.cpp \
           src/io/AnsiColors.cpp \
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
           src/net/TcpProxy.cpp \
           src/plugin/Plugin.cpp \
           src/plugin/ShellPlugin.cpp \
           src/plugin/ShellPluginInfo.cpp \
           src/plugin/info/InfoPlugin.cpp \
           src/plugin/info/InfoPluginFactory.cpp \
           src/plugins/info/TestPlugin.cpp \
           src/statistics/LaunchMethodStatistics.cpp \
           src/statistics/PilotLog.cpp \
           src/statistics/PlaneLog.cpp \
           src/util/bool.cpp \
           src/util/qList.cpp \
           src/util/qString.cpp \
           src/util/time.cpp \
		   # Empty line

FORMS += \
           src/gui/windows/DateInputDialog.ui \
           src/gui/windows/FlightWindow.ui \
           src/gui/windows/LaunchMethodSelectionWindow.ui \
           src/gui/windows/MainWindow.ui \
           src/gui/windows/MonitorDialog.ui \
           src/gui/windows/ObjectListWindowBase.ui \
           src/gui/windows/ObjectSelectWindowBase.ui \
           src/gui/windows/SettingsWindow.ui \
           src/gui/windows/StatisticsWindow.ui \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.ui \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.ui \
           src/gui/windows/objectEditor/PersonEditorPane.ui \
           src/gui/windows/objectEditor/PlaneEditorPane.ui \
           # Empty line

RESOURCES += \
           startkladde.qrc \
           # Empty line

# Windows resource file (e. g. exe file icon)
RC_FILE = startkladde.rc

# ACPI widget: on windows, use the dummy implementation, on unix, use the
# libacpi implementation
win32 {
	SOURCES += src/gui/widgets/AcpiWidget_dummy.cpp
}
unix {
	SOURCES += src/gui/widgets/AcpiWidget_libacpi.cpp
	LIBS += -lacpi
}
# To compile without ACPI support, remove the block above and
# enable the following line: 
#SOURCES += src/gui/widgets/AcpiWidget_dummy.cpp

# Include the file defining the migration input files (autogenerated)
!include( build/migrations.pro ) {
error( "build/migrations.pro could not be included" )
}


##################
## Installation ##
##################

# CONFIG += copy_dir_files

unix {
	target.path +=  /usr/bin/
	# target.files does not have to be specified explicitly

	plugins.path += /usr/lib/startkladde/
	plugins.files += plugins

	menu.path += /usr/share/applications/
	menu.files += startkladde.desktop

	icon.path += /usr/share/pixmaps
	icon.files += graphics/startkladde.png

	INSTALLS += target plugins menu icon
}

