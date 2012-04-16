cmake_minimum_required(VERSION 2.8)

PROJECT(startkladdde)
FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTNETWORK TRUE)

set(CMAKE_AUTOMOC TRUE)

SET(startkladde_SOURCES
		   build/*.cpp
           src/*.cpp
           src/concurrent/*.cpp
           src/config/*.cpp
           src/data/*.cpp
           src/db/*.cpp
           src/db/cache/*.cpp
           src/db/event/*.cpp
           src/db/interface/*.cpp
           src/db/interface/exceptions/*.cpp
           src/db/migration/*.cpp
           src/db/result/*.cpp
           src/db/schema/*.cpp
           src/db/schema/spec/*.cpp
           src/graphics/*.cpp
           src/gui/*.cpp
           src/gui/views/*.cpp
           src/gui/widgets/*.cpp
           src/gui/windows/*.cpp
           src/i18n/*.cpp
           src/io/*.cpp
           src/logging/*.cpp
           src/model/*.cpp
           src/model/flightList/*.cpp
           src/net/*.cpp
           src/plugin/*.cpp
           src/plugin/factory/*.cpp
           src/plugin/info/*.cpp
           src/plugin/settings/*.cpp
           src/plugin/weather/*.cpp
           src/plugins/info/external/*.cpp
           src/plugins/info/metar/*.cpp
           src/plugins/info/sunset/*.cpp
           src/plugins/info/test/*.cpp
           src/plugins/weather/*.cpp
           src/statistics/*.cpp
           src/statistics/*.cpp
           src/util/*.cpp
)
#SET(startkladde_HEADERS hellowindow.h)
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


#QT4_WRAP_CPP(startkladde_HEADERS_MOC ${startkladde_HEADERS})
QT4_WRAP_UI(startkladde_FORMS_HEADERS ${startkladde_FORMS})
QT4_ADD_RESOURCES(startkladde_RESOURCES_RCC ${startkladde_RESOURCES})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(startkladde ${startkladde_SOURCES} 
	#    ${startkladde_HEADERS_MOC} 
    ${startkladde_FORMS_HEADERS} 
    ${startkladde_RESOURCES_RCC})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


TARGET_LINK_LIBRARIES(startkladde ${QT_LIBRARIES})


