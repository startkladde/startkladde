##############
## Settings ##
##############

# Compiling on Windows natively:
win32: MYSQL_INCLUDE_PATH = 'c:/program files (x86)/mysql/mysql server 5.5/include'
# Cross compiling:
#win32: MYSQL_INCLUDE_PATH = /usr/include/mysql


#########################
## Build configuration ##
#########################

TEMPLATE = app
CONFIG += qt thread resources rtti
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
QMAKE_CXXFLAGS += -Wextra

win32:DEFINES += SK_WINDOWS
unix:DEFINES += SK_UNIX


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
	# /usr/lib/mysql and /usr/lib64/mysql is for Fedora
	LIBS += -L/usr/lib/mysql -L/usr/lib64/mysql -lmysqlclient
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
           src/flightColor.h \
           src/Longitude.h \
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
           src/container/SkMultiHash.h \
           src/container/SortedSet.h \
           src/container/SortedSet_impl.h \
           src/data/Csv.h \
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
           src/db/migration/MigrationBuilder.h \
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
           src/graphics/SkMovie.h \
           src/gui/dialogs.h \
           src/gui/PasswordCheck.h \
           src/gui/PasswordPermission.h \
		   src/gui/SkDialog.h \
		   src/gui/SkMainWindow.h \
           src/gui/views/ReadOnlyItemDelegate.h \
           src/gui/views/SkItemDelegate.h \
           src/gui/views/SpecialIntDelegate.h \
           src/gui/views/SpinBoxCreator.h \
           src/gui/widgets/AcpiWidget.h \
           src/gui/widgets/LanguageComboBox.h \
           src/gui/widgets/LongitudeInput.h \
           src/gui/widgets/SkComboBox.h \
           src/gui/widgets/SkLabel.h \
           src/gui/widgets/SkTableView.h \
           src/gui/widgets/SkTreeWidgetItem.h \
           src/gui/widgets/TableButton.h \
           src/gui/widgets/WeatherWidget.h \
           src/gui/windows/AboutDialog.h \
           src/gui/windows/ConfirmOverwritePersonDialog.h \
           src/gui/windows/CsvExportDialog.h \
           src/gui/windows/input/DateInputDialog.h \
           src/gui/windows/input/DateTimeInputDialog.h \
           src/gui/windows/FlightWindow.h \
           src/gui/windows/LaunchMethodSelectionWindow.h \
           src/gui/windows/MainWindow.h \
           src/gui/windows/MonitorDialog.h \
           src/gui/windows/ObjectSelectWindow.h \
           src/gui/windows/ObjectSelectWindowBase.h \
           src/gui/windows/SettingsWindow.h \
           src/gui/windows/StatisticsWindow.h \
           src/gui/windows/WeatherDialog.h \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorPane.h \
           src/gui/windows/objectEditor/ObjectEditorWindow.h \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.h \
           src/gui/windows/objectEditor/PersonEditorPane.h \
           src/gui/windows/objectEditor/PlaneEditorPane.h \
           src/gui/windows/objectList/FlightListWindow.h \
           src/gui/windows/objectList/ObjectListWindow.h \
           src/gui/windows/objectList/PersonListWindow.h \
           src/gui/windows/objectList/ObjectListWindowBase.h \
           src/i18n/LanguageChangeNotifier.h \
           src/i18n/LanguageConfiguration.h \
           src/i18n/notr.h \
           src/i18n/TranslationManager.h \
           src/io/AnsiColors.h \
           src/io/SkProcess.h \
           src/logging/messages.h \
           src/model/Entity.h \
           src/model/Flight.h \
           src/model/FlightBase.h \
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
           src/net/Downloader.h \
           src/net/Network.h \
           src/net/TcpProxy.h \
           src/plugin/Plugin.h \
           src/plugin/factory/PluginFactory.h \
           src/plugin/info/InfoPlugin.h \
           src/plugin/info/InfoPluginSelectionDialog.h \
           src/plugin/info/InfoPluginSettingsPane.h \
           src/plugin/settings/PluginSettingsPane.h \
           src/plugin/settings/PluginSettingsDialog.h \
           src/plugin/weather/WeatherPlugin.h \
           src/plugins/info/external/ExternalInfoPlugin.h \
           src/plugins/info/external/ExternalInfoPluginSettingsPane.h \
           src/plugins/info/metar/MetarPlugin.h \
           src/plugins/info/metar/MetarPluginSettingsPane.h \
           src/plugins/info/sunset/SunsetPluginBase.h \
           src/plugins/info/sunset/SunsetCountdownPlugin.h \
           src/plugins/info/sunset/SunsetTimePlugin.h \
           src/plugins/info/sunset/SunsetPluginSettingsPane.h \
           src/plugins/info/test/TestPlugin.h \
           src/plugins/info/test/TestPluginSettingsPane.h \
           src/plugins/weather/ExternalWeatherPlugin.h \
           src/plugins/weather/WetterOnlineAnimationPlugin.h \
           src/plugins/weather/WetterOnlineImagePlugin.h \
           src/statistics/LaunchMethodStatistics.h \
           src/statistics/PilotLog.h \
           src/statistics/PlaneLog.h \
           src/util/bool.h \
           src/util/color.h \
           src/util/environment.h \
           src/util/file.h \
           src/util/io.h \
           src/util/qDate.h \
           src/util/qList.h \
           src/util/qString.h \
           src/util/time.h \
		   # Empty line

SOURCES += \
		   build/CurrentSchema.cpp \
           src/flightColor.cpp \
           src/Longitude.cpp \
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
           src/data/Csv.cpp \
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
           src/graphics/SkMovie.cpp \
           src/gui/dialogs.cpp \
           src/gui/PasswordCheck.cpp \
           src/gui/PasswordPermission.cpp \
           src/gui/views/ReadOnlyItemDelegate.cpp \
           src/gui/views/SkItemDelegate.cpp \
           src/gui/views/SpecialIntDelegate.cpp \
           src/gui/views/SpinBoxCreator.cpp \
           src/gui/widgets/LanguageComboBox.cpp \
           src/gui/widgets/LongitudeInput.cpp \
           src/gui/widgets/SkComboBox.cpp \
           src/gui/widgets/SkLabel.cpp \
           src/gui/widgets/SkTableView.cpp \
           src/gui/widgets/SkTreeWidgetItem.cpp \
           src/gui/widgets/TableButton.cpp \
           src/gui/widgets/WeatherWidget.cpp \
           src/gui/windows/ConfirmOverwritePersonDialog.cpp \
           src/gui/windows/AboutDialog.cpp \
           src/gui/windows/CsvExportDialog.cpp \
           src/gui/windows/input/DateInputDialog.cpp \
           src/gui/windows/input/DateTimeInputDialog.cpp \
           src/gui/windows/FlightWindow.cpp \
           src/gui/windows/LaunchMethodSelectionWindow.cpp \
           src/gui/windows/MainWindow.cpp \
           src/gui/windows/MonitorDialog.cpp \
           src/gui/windows/ObjectSelectWindow.cpp \
           src/gui/windows/ObjectSelectWindowBase.cpp \
           src/gui/windows/SettingsWindow.cpp \
           src/gui/windows/StatisticsWindow.cpp \
           src/gui/windows/WeatherDialog.cpp \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorPane.cpp \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.cpp \
           src/gui/windows/objectEditor/PersonEditorPane.cpp \
           src/gui/windows/objectEditor/PlaneEditorPane.cpp \
           src/gui/windows/objectList/FlightListWindow.cpp \
           src/gui/windows/objectList/ObjectListWindow.cpp \
           src/gui/windows/objectList/PersonListWindow.cpp \
           src/gui/windows/objectList/ObjectListWindowBase.cpp \
           src/i18n/LanguageChangeNotifier.cpp \
           src/i18n/LanguageConfiguration.cpp \
           src/i18n/TranslationManager.cpp \
           src/io/AnsiColors.cpp \
           src/io/SkProcess.cpp \
           src/logging/messages.cpp \
           src/model/Entity.cpp \
           src/model/Flight.cpp \
           src/model/FlightBase.cpp \
           src/model/Flight_Mode.cpp \
           src/model/Flight_Type.cpp \
           src/model/LaunchMethod.cpp \
           src/model/Person.cpp \
           src/model/PersonModel.cpp \
           src/model/Plane.cpp \
           src/model/flightList/FlightModel.cpp \
           src/model/flightList/FlightProxyList.cpp \
           src/model/flightList/FlightSortFilterProxyModel.cpp \
           src/net/Downloader.cpp \
           src/net/Network.cpp \
           src/net/TcpProxy.cpp \
           src/plugin/Plugin.cpp \
           src/plugin/factory/PluginFactory.cpp \
           src/plugin/info/InfoPlugin.cpp \
           src/plugin/info/InfoPluginSelectionDialog.cpp \
           src/plugin/info/InfoPluginSettingsPane.cpp \
           src/plugin/settings/PluginSettingsPane.cpp \
           src/plugin/settings/PluginSettingsDialog.cpp \
           src/plugin/weather/WeatherPlugin.cpp \
           src/plugins/info/external/ExternalInfoPlugin.cpp \
           src/plugins/info/external/ExternalInfoPluginSettingsPane.cpp \
           src/plugins/info/metar/MetarPlugin.cpp \
           src/plugins/info/metar/MetarPluginSettingsPane.cpp \
           src/plugins/info/sunset/SunsetPluginBase.cpp \
           src/plugins/info/sunset/SunsetCountdownPlugin.cpp \
           src/plugins/info/sunset/SunsetTimePlugin.cpp \
           src/plugins/info/sunset/SunsetPluginSettingsPane.cpp \
           src/plugins/info/test/TestPlugin.cpp \
           src/plugins/info/test/TestPluginSettingsPane.cpp \
           src/plugins/weather/ExternalWeatherPlugin.cpp \
           src/plugins/weather/WetterOnlineAnimationPlugin.cpp \
           src/plugins/weather/WetterOnlineImagePlugin.cpp \
           src/statistics/LaunchMethodStatistics.cpp \
           src/statistics/PilotLog.cpp \
           src/statistics/PlaneLog.cpp \
           src/util/bool.cpp \
           src/util/color.cpp \
           src/util/environment.cpp \
           src/util/file.cpp \
           src/util/io.cpp \
           src/util/qDate.cpp \
           src/util/qString.cpp \
           src/util/time.cpp \
		   # Empty line

FORMS += \
           src/gui/widgets/LongitudeInput.ui \
           src/gui/windows/AboutDialog.ui \
           src/gui/windows/ConfirmOverwritePersonDialog.ui \
           src/gui/windows/CsvExportDialog.ui \
           src/gui/windows/input/DateInputDialog.ui \
           src/gui/windows/input/DateTimeInputDialog.ui \
           src/gui/windows/FlightWindow.ui \
           src/gui/windows/LaunchMethodSelectionWindow.ui \
           src/gui/windows/MainWindow.ui \
           src/gui/windows/MonitorDialog.ui \
           src/gui/windows/ObjectSelectWindowBase.ui \
           src/gui/windows/SettingsWindow.ui \
           src/gui/windows/StatisticsWindow.ui \
           src/gui/windows/objectEditor/LaunchMethodEditorPane.ui \
           src/gui/windows/objectEditor/ObjectEditorWindowBase.ui \
           src/gui/windows/objectEditor/PersonEditorPane.ui \
           src/gui/windows/objectEditor/PlaneEditorPane.ui \
           src/gui/windows/objectList/FlightListWindow.ui \
           src/gui/windows/objectList/ObjectListWindowBase.ui \
           src/plugin/info/InfoPluginSelectionDialog.ui \
           src/plugin/info/InfoPluginSettingsPane.ui \
           src/plugin/settings/PluginSettingsDialog.ui \
           src/plugins/info/external/ExternalInfoPluginSettingsPane.ui \
           src/plugins/info/metar/MetarPluginSettingsPane.ui \
           src/plugins/info/sunset/SunsetPluginSettingsPane.ui \
           src/plugins/info/test/TestPluginSettingsPane.ui \
           # Empty line

RESOURCES += \
           config/startkladde.qrc \
           # Empty line

# Windows resource file (e. g. exe file icon)
RC_FILE = config/startkladde.rc

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

TRANSLATIONS += translations/startkladde_de.ts translations/startkladde_bork.ts

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
	menu.files += config/startkladde.desktop

	icon.path += /usr/share/pixmaps
	icon.files += graphics/startkladde.png

	INSTALLS += target plugins menu icon
}

