# TODO add to version: svn revision
# FIXME svnversion statt revision aus svn info?
# TODO handle svn not found, not a working copy
# TODO copy translations
# TODO use globbing for sources and headers (and remove the inappropriate acpi widget)
# TODO SK_WINDOWS on Windows, or individually configure the values it is used
# for
# TODO use RELEASE as default build type?
# TODO the following compiler options and defines were used by qmake:
#   -Wextra -O2 -frtti -fexceptions -mthreads -Wall
#   -DQT_LARGEFILE_SUPPORT -DQT_NEEDS_QMAIN -DQT_THREAD_SUPPORT -DUNICODE
#   -DQT_HAVE_MMX -DQT_HAVE_3DNOW -DQT_HAVE_SSE -DQT_HAVE_MMXEXT -DQT_HAVE_SSE2
# TODO clean up compiler options, add -Wall
# TODO installation (cpack): include required Qt plugins (Windows only) and translations

cmake_minimum_required (VERSION 2.8)
PROJECT (startkladdde)

# Include additional cmake scripts (e. g. finding the MySQL library)
SET (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")


##############
## Packages ##
##############

# Find packages
FIND_PACKAGE (Qt4            REQUIRED)
FIND_PACKAGE (MySQL      5.0 REQUIRED)
FIND_PACKAGE (Subversion 1.4)

# Setup the Qt library
SET(QT_USE_QTSQL     TRUE)
SET(QT_USE_QTNETWORK TRUE)

# This is available starting 2.8.6 which is not currently available in the
# latest Ubuntu release.
#set(CMAKE_AUTOMOC TRUE)

# For gcc and release node, enable some options that make the binary smaller
if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        -DQT_NO_DEBUG") # Disable QT debug code
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s"           ) # Strip binary
endif ()





##################
## Source files ##
##################

file (GLOB MIGRATIONS src/db/migrations/Migration_*.cpp)

SET(startkladde_SOURCES
	${MIGRATIONS}
		   build/CurrentSchema.cpp
           src/flightColor.cpp
           src/Longitude.cpp
           src/startkladde.cpp
           src/StorableException.cpp
           src/text.cpp
           src/version.cpp
           src/concurrent/DefaultQThread.cpp
           src/concurrent/Returner.cpp
           src/concurrent/synchronized.cpp
           src/concurrent/threadUtil.cpp
           src/concurrent/Waiter.cpp
           src/concurrent/monitor/OperationCanceledException.cpp
           src/concurrent/monitor/OperationMonitor.cpp
           src/concurrent/monitor/OperationMonitorInterface.cpp
           src/concurrent/monitor/SignalOperationMonitor.cpp
           src/concurrent/monitor/SimpleOperationMonitor.cpp
           src/config/Settings.cpp
           src/data/Csv.cpp
           src/db/Database.cpp
           src/db/DatabaseInfo.cpp
           src/db/DbManager.cpp
           src/db/DbWorker.cpp
           src/db/Query.cpp
           src/db/dbId.cpp
           src/db/cache/Cache.cpp
           src/db/cache/Cache_hashUpdates.cpp
           src/db/cache/Cache_lookup.cpp
           src/db/cache/CacheWorker.cpp
           src/db/event/DbEvent.cpp
           src/db/event/DbEventMonitor.cpp
           src/db/interface/AbstractInterface.cpp
           src/db/interface/DefaultInterface.cpp
           src/db/interface/Interface.cpp
           src/db/interface/InterfaceWorker.cpp
           src/db/interface/ThreadSafeInterface.cpp
           src/db/interface/exceptions/ConnectionFailedException.cpp
           src/db/interface/exceptions/PingFailedException.cpp
           src/db/interface/exceptions/QueryFailedException.cpp
           src/db/interface/exceptions/TransactionFailedException.cpp
           src/db/interface/exceptions/SqlException.cpp
           src/db/interface/exceptions/AccessDeniedException.cpp
           src/db/interface/exceptions/DatabaseDoesNotExistException.cpp
           src/db/migration/Migration.cpp
           src/db/migration/MigrationFactory.cpp
           src/db/migration/Migrator.cpp
           src/db/migration/MigratorWorker.cpp
           src/db/result/CopiedResult.cpp
           src/db/result/DefaultResult.cpp
           src/db/schema/Schema.cpp
           src/db/schema/SchemaDumper.cpp
           src/db/schema/spec/ColumnSpec.cpp
           src/db/schema/spec/IndexSpec.cpp
           src/graphics/SkMovie.cpp
           src/gui/dialogs.cpp
           src/gui/PasswordCheck.cpp
           src/gui/PasswordPermission.cpp
           src/gui/views/ReadOnlyItemDelegate.cpp
           src/gui/views/SkItemDelegate.cpp
           src/gui/views/SpecialIntDelegate.cpp
           src/gui/views/SpinBoxCreator.cpp
           src/gui/widgets/LanguageComboBox.cpp
           src/gui/widgets/LongitudeInput.cpp
           src/gui/widgets/SkComboBox.cpp
           src/gui/widgets/SkLabel.cpp
           src/gui/widgets/SkTableView.cpp
           src/gui/widgets/SkTreeWidgetItem.cpp
           src/gui/widgets/TableButton.cpp
           src/gui/widgets/WeatherWidget.cpp
           src/gui/windows/ConfirmOverwritePersonDialog.cpp
           src/gui/windows/AboutDialog.cpp
           src/gui/windows/CsvExportDialog.cpp
           src/gui/windows/input/DateInputDialog.cpp
           src/gui/windows/input/DateTimeInputDialog.cpp
           src/gui/windows/FlightWindow.cpp
           src/gui/windows/LaunchMethodSelectionWindow.cpp
           src/gui/windows/MainWindow.cpp
           src/gui/windows/MonitorDialog.cpp
           src/gui/windows/ObjectSelectWindow.cpp
           src/gui/windows/ObjectSelectWindowBase.cpp
           src/gui/windows/SettingsWindow.cpp
           src/gui/windows/StatisticsWindow.cpp
           src/gui/windows/WeatherDialog.cpp
           src/gui/windows/objectEditor/LaunchMethodEditorPane.cpp
           src/gui/windows/objectEditor/ObjectEditorPane.cpp
           src/gui/windows/objectEditor/ObjectEditorWindowBase.cpp
           src/gui/windows/objectEditor/PersonEditorPane.cpp
           src/gui/windows/objectEditor/PlaneEditorPane.cpp
           src/gui/windows/objectList/FlightListWindow.cpp
           src/gui/windows/objectList/ObjectListWindow.cpp
           src/gui/windows/objectList/PersonListWindow.cpp
           src/gui/windows/objectList/ObjectListWindowBase.cpp
           src/i18n/LanguageChangeNotifier.cpp
           src/i18n/LanguageConfiguration.cpp
           src/i18n/TranslationManager.cpp
           src/io/AnsiColors.cpp
           src/io/SkProcess.cpp
           src/logging/messages.cpp
           src/model/Entity.cpp
           src/model/Flight.cpp
           src/model/FlightBase.cpp
           src/model/Flight_Mode.cpp
           src/model/Flight_Type.cpp
           src/model/LaunchMethod.cpp
           src/model/Person.cpp
           src/model/PersonModel.cpp
           src/model/Plane.cpp
           src/model/flightList/FlightModel.cpp
           src/model/flightList/FlightProxyList.cpp
           src/model/flightList/FlightSortFilterProxyModel.cpp
           src/net/Downloader.cpp
           src/net/Network.cpp
           src/net/TcpProxy.cpp
           src/plugin/Plugin.cpp
           src/plugin/factory/PluginFactory.cpp
           src/plugin/info/InfoPlugin.cpp
           src/plugin/info/InfoPluginSelectionDialog.cpp
           src/plugin/info/InfoPluginSettingsPane.cpp
           src/plugin/settings/PluginSettingsPane.cpp
           src/plugin/settings/PluginSettingsDialog.cpp
           src/plugin/weather/WeatherPlugin.cpp
           src/plugins/info/external/ExternalInfoPlugin.cpp
           src/plugins/info/external/ExternalInfoPluginSettingsPane.cpp
           src/plugins/info/metar/MetarPlugin.cpp
           src/plugins/info/metar/MetarPluginSettingsPane.cpp
           src/plugins/info/sunset/SunsetPluginBase.cpp
           src/plugins/info/sunset/SunsetCountdownPlugin.cpp
           src/plugins/info/sunset/SunsetTimePlugin.cpp
           src/plugins/info/sunset/SunsetPluginSettingsPane.cpp
           src/plugins/info/test/TestPlugin.cpp
           src/plugins/info/test/TestPluginSettingsPane.cpp
           src/plugins/weather/ExternalWeatherPlugin.cpp
           src/plugins/weather/WetterOnlineAnimationPlugin.cpp
           src/plugins/weather/WetterOnlineImagePlugin.cpp
           src/statistics/LaunchMethodStatistics.cpp
           src/statistics/PilotLog.cpp
           src/statistics/PlaneLog.cpp
           src/util/bool.cpp
           src/util/color.cpp
           src/util/environment.cpp
           src/util/file.cpp
           src/util/io.cpp
           src/util/qDate.cpp
           src/util/qString.cpp
           src/util/time.cpp
		   #		   src/gui/widgets/AcpiWidget_libacpi.cpp #linux FIXME, and
		   #		   -lacpi
		   src/gui/widgets/AcpiWidget_dummy.cpp #windows
)

# FIXME translations

SET(startkladde_HEADERS
           src/accessor.h
           src/flightColor.h
           src/Longitude.h
           src/StorableException.h
           src/text.h
           src/version.h
           src/concurrent/DefaultQThread.h
           src/concurrent/Returner.h
           src/concurrent/synchronized.h
           src/concurrent/threadUtil.h
           src/concurrent/Waiter.h
           src/concurrent/monitor/OperationCanceledException.h
           src/concurrent/monitor/OperationMonitor.h
           src/concurrent/monitor/OperationMonitorInterface.h
           src/concurrent/monitor/SignalOperationMonitor.h
           src/concurrent/monitor/SimpleOperationMonitor.h
           src/config/Settings.h
           src/container/SkMultiHash.h
           src/container/SortedSet.h
           src/container/SortedSet_impl.h
           src/data/Csv.h
           src/db/Database.h
           src/db/DbManager.h
           src/db/DbWorker.h
           src/db/Query.h
           src/db/dbId.h
           src/db/cache/Cache.h
           src/db/cache/CacheWorker.h
           src/db/event/DbEvent.h
           src/db/event/DbEventMonitor.h
           src/db/interface/AbstractInterface.h
           src/db/interface/DefaultInterface.h
           src/db/interface/Interface.h
           src/db/interface/InterfaceWorker.h
           src/db/interface/ThreadSafeInterface.h
           src/db/interface/exceptions/ConnectionFailedException.h
           src/db/interface/exceptions/PingFailedException.h
           src/db/interface/exceptions/QueryFailedException.h
           src/db/interface/exceptions/TransactionFailedException.h
           src/db/interface/exceptions/SqlException.h
           src/db/interface/exceptions/AccessDeniedException.h
           src/db/interface/exceptions/DatabaseDoesNotExistException.h
           src/db/migration/Migration.h
           src/db/migration/MigrationBuilder.h
           src/db/migration/MigrationFactory.h
           src/db/migration/Migrator.h
           src/db/migration/MigratorWorker.h
           src/db/result/CopiedResult.h
           src/db/result/DefaultResult.h
           src/db/result/Result.h
           src/db/schema/CurrentSchema.h
           src/db/schema/Schema.h
           src/db/schema/SchemaDumper.h
           src/db/schema/spec/ColumnSpec.h
           src/db/schema/spec/IndexSpec.h
           src/graphics/SkMovie.h
           src/gui/dialogs.h
           src/gui/PasswordCheck.h
           src/gui/PasswordPermission.h
		   src/gui/SkDialog.h
		   src/gui/SkMainWindow.h
           src/gui/views/ReadOnlyItemDelegate.h
           src/gui/views/SkItemDelegate.h
           src/gui/views/SpecialIntDelegate.h
           src/gui/views/SpinBoxCreator.h
           src/gui/widgets/AcpiWidget.h
           src/gui/widgets/LanguageComboBox.h
           src/gui/widgets/LongitudeInput.h
           src/gui/widgets/SkComboBox.h
           src/gui/widgets/SkLabel.h
           src/gui/widgets/SkTableView.h
           src/gui/widgets/SkTreeWidgetItem.h
           src/gui/widgets/TableButton.h
           src/gui/widgets/WeatherWidget.h
           src/gui/windows/AboutDialog.h
           src/gui/windows/ConfirmOverwritePersonDialog.h
           src/gui/windows/CsvExportDialog.h
           src/gui/windows/input/DateInputDialog.h
           src/gui/windows/input/DateTimeInputDialog.h
           src/gui/windows/FlightWindow.h
           src/gui/windows/LaunchMethodSelectionWindow.h
           src/gui/windows/MainWindow.h
           src/gui/windows/MonitorDialog.h
           src/gui/windows/ObjectSelectWindow.h
           src/gui/windows/ObjectSelectWindowBase.h
           src/gui/windows/SettingsWindow.h
           src/gui/windows/StatisticsWindow.h
           src/gui/windows/WeatherDialog.h
           src/gui/windows/objectEditor/LaunchMethodEditorPane.h
           src/gui/windows/objectEditor/ObjectEditorPane.h
           src/gui/windows/objectEditor/ObjectEditorWindow.h
           src/gui/windows/objectEditor/ObjectEditorWindowBase.h
           src/gui/windows/objectEditor/PersonEditorPane.h
           src/gui/windows/objectEditor/PlaneEditorPane.h
           src/gui/windows/objectList/FlightListWindow.h
           src/gui/windows/objectList/ObjectListWindow.h
           src/gui/windows/objectList/PersonListWindow.h
           src/gui/windows/objectList/ObjectListWindowBase.h
           src/i18n/LanguageChangeNotifier.h
           src/i18n/LanguageConfiguration.h
           src/i18n/notr.h
           src/i18n/TranslationManager.h
           src/io/AnsiColors.h
           src/io/SkProcess.h
           src/logging/messages.h
           src/model/Entity.h
           src/model/Flight.h
           src/model/FlightBase.h
           src/model/LaunchMethod.h
           src/model/Person.h
           src/model/Plane.h
           src/model/flightList/FlightModel.h
           src/model/flightList/FlightProxyList.h
           src/model/flightList/FlightSortFilterProxyModel.h
           src/model/objectList/AbstractObjectList.h
           src/model/objectList/AutomaticEntityList.h
           src/model/objectList/ColumnInfo.h
           src/model/objectList/EntityList.h
           src/model/objectList/MutableObjectList.h
           src/model/objectList/ObjectListModel.h
           src/model/objectList/ObjectModel.h
           src/net/Downloader.h
           src/net/Network.h
           src/net/TcpProxy.h
           src/plugin/Plugin.h
           src/plugin/factory/PluginFactory.h
           src/plugin/info/InfoPlugin.h
           src/plugin/info/InfoPluginSelectionDialog.h
           src/plugin/info/InfoPluginSettingsPane.h
           src/plugin/settings/PluginSettingsPane.h
           src/plugin/settings/PluginSettingsDialog.h
           src/plugin/weather/WeatherPlugin.h
           src/plugins/info/external/ExternalInfoPlugin.h
           src/plugins/info/external/ExternalInfoPluginSettingsPane.h
           src/plugins/info/metar/MetarPlugin.h
           src/plugins/info/metar/MetarPluginSettingsPane.h
           src/plugins/info/sunset/SunsetPluginBase.h
           src/plugins/info/sunset/SunsetCountdownPlugin.h
           src/plugins/info/sunset/SunsetTimePlugin.h
           src/plugins/info/sunset/SunsetPluginSettingsPane.h
           src/plugins/info/test/TestPlugin.h
           src/plugins/info/test/TestPluginSettingsPane.h
           src/plugins/weather/ExternalWeatherPlugin.h
           src/plugins/weather/WetterOnlineAnimationPlugin.h
           src/plugins/weather/WetterOnlineImagePlugin.h
           src/statistics/LaunchMethodStatistics.h
           src/statistics/PilotLog.h
           src/statistics/PlaneLog.h
           src/util/bool.h
           src/util/color.h
           src/util/environment.h
           src/util/file.h
           src/util/io.h
           src/util/qDate.h
           src/util/qList.h
           src/util/qString.h
           src/util/time.h
		   )


SET(startkladde_FORMS
           src/gui/widgets/LongitudeInput.ui
           src/gui/windows/AboutDialog.ui
           src/gui/windows/ConfirmOverwritePersonDialog.ui
           src/gui/windows/CsvExportDialog.ui
           src/gui/windows/input/DateInputDialog.ui
           src/gui/windows/input/DateTimeInputDialog.ui
           src/gui/windows/FlightWindow.ui
           src/gui/windows/LaunchMethodSelectionWindow.ui
           src/gui/windows/MainWindow.ui
           src/gui/windows/MonitorDialog.ui
           src/gui/windows/ObjectSelectWindowBase.ui
           src/gui/windows/SettingsWindow.ui
           src/gui/windows/StatisticsWindow.ui
           src/gui/windows/objectEditor/LaunchMethodEditorPane.ui
           src/gui/windows/objectEditor/ObjectEditorWindowBase.ui
           src/gui/windows/objectEditor/PersonEditorPane.ui
           src/gui/windows/objectEditor/PlaneEditorPane.ui
           src/gui/windows/objectList/FlightListWindow.ui
           src/gui/windows/objectList/ObjectListWindowBase.ui
           src/plugin/info/InfoPluginSelectionDialog.ui
           src/plugin/info/InfoPluginSettingsPane.ui
           src/plugin/settings/PluginSettingsDialog.ui
           src/plugins/info/external/ExternalInfoPluginSettingsPane.ui
           src/plugins/info/metar/MetarPluginSettingsPane.ui
           src/plugins/info/sunset/SunsetPluginSettingsPane.ui
           src/plugins/info/test/TestPluginSettingsPane.ui
)
SET(startkladde_RESOURCES 
	config/startkladde.qrc
)


QT4_WRAP_CPP(startkladde_HEADERS_MOC ${startkladde_HEADERS})
QT4_WRAP_UI(startkladde_FORMS_HEADERS ${startkladde_FORMS})
QT4_ADD_RESOURCES(startkladde_RESOURCES_RCC ${startkladde_RESOURCES})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

#add_custom_command(
#      OUTPUT ${PROJECT_BINARY_DIR}/sk.bat
#      COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/sk.bat ${PROJECT_BINARY_DIR}/sk.bat
#      DEPENDS ${PROJECT_SOURCE_DIR}/sk.bat 
#      )

if(WIN32)
	# TODO do this on or after build, not every time we run cmake.
	# add_custom_command with add_custom_target may or may not be appropriate. 
	SET (qtlibs QtCore4.dll QtGui4.dll QtSql4.dll QtNetwork4.dll mingwm10.dll libgcc_s_dw2-1.dll)
	foreach (qtlib ${qtlibs})
		message ("Pulling in Qt library ${qtlib}")
		file (COPY ${QT_BINARY_DIR}/${qtlib} DESTINATION ${PROJECT_BINARY_DIR})
	endforeach()

	# TODO the MySQL finder should export MYSQL_LIBRARY_DIR or MYSQL_BINARY_DIR
	message ("Pulling in MySQL library libmysql.dll")
	file (COPY ${MYSQL_INCLUDE_DIR}/../lib/libmysql.dll DESTINATION ${PROJECT_BINARY_DIR})
	
	SET (startkladde_WINDOWS_RESOURCES config/startkladde.rc)
endif()

set (startkladde_VERSION_MAJOR 2)
set (startkladde_VERSION_MINOR 1)
set (startkladde_VERSION_REVISION 0)

configure_file ("${PROJECT_SOURCE_DIR}/src/build.h.in" "${PROJECT_BINARY_DIR}/build.h")

include_directories("${PROJECT_BINARY_DIR}")
	
ADD_EXECUTABLE(startkladde
	WIN32                            # GUI application on Windows - ignored on other platforms
	${startkladde_SOURCES} 
	${startkladde_HEADERS_MOC} 
    ${startkladde_FORMS_HEADERS} 
    ${startkladde_RESOURCES_RCC}
    ${startkladde_WINDOWS_RESOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${MYSQL_INCLUDE_DIR})


TARGET_LINK_LIBRARIES(startkladde ${QT_LIBRARIES})


#########
## SVN ##
#########

## a custom target that is always built
#
## creates svnheader.h using cmake script

# a custom target that is always built
#add_custom_target (svnheader ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h)
ADD_CUSTOM_TARGET(svnheader ALL)

# creates svnversion.h using cmake script
#add_custom_command (
#	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
#    COMMAND ${CMAKE_COMMAND}
#    	-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
#    	-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generateSvnHeader.cmake
#)
ADD_CUSTOM_COMMAND(
	TARGET svnheader
	COMMAND ${CMAKE_COMMAND}
		-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} 
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generateSvnHeader.cmake
)


# svnVersion.h is a generated file
set_source_files_properties (
	${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

# explicitly say that the executable depends on the svnheader
add_dependencies (startkladde svnheader)
