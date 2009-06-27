CONFIG += qt debug
QT += qt3support
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi
MAKEFILE = Makefile_startkladde
OBJECTS_DIR = build/
MOC_DIR= build/

# Input
HEADERS += \
		   kvkbd.xpm \
		   logo.xpm \
           src/accessor.h \
           src/color.h \
           src/Condition.h \
           src/dataTypes.h \
           src/FlightList.h \
           src/ObjectField.h \
           src/SkException.h \
           src/text.h \
           src/version.h \
           src/config/Options.h \
           src/db/adminFunctions.h \
           src/db/DbColumn.h \
           src/db/DbEvent.h \
           src/db/dbProxy.h \
           src/db/DbTable.h \
           src/db/dbTypes.h \
           src/db/Database.h \
           src/documents/LatexDocument.h \
           src/documents/Table.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/gui/spacing.h \
           src/gui/widgets/AcpiWidget.h \
           src/gui/widgets/FlightTable.h \
           src/gui/widgets/SkButton.h \
           src/gui/widgets/SkComboBox.h \
           src/gui/widgets/SkLabel.h \
           src/gui/widgets/SkListWidget.h \
           src/gui/widgets/SkTable.h \
           src/gui/widgets/SkTableItem.h \
           src/gui/widgets/SkTextBox.h \
           src/gui/widgets/WeatherWidget.h \
           src/gui/widgets/LabelComboBox.h \
           src/gui/widgets/SkTreeWidgetItem.h \
           src/gui/widgets/SkTimeEdit.h \
           src/gui/windows/DateWindow.h \
           src/gui/windows/FlightWindow.h \
           src/gui/windows/MainWindow.h \
           src/gui/windows/SkDialog.h \
           src/gui/windows/SplashScreen.h \
           src/gui/windows/StatisticsWindow.h \
           src/gui/windows/EntityEditWindow.h \
           src/gui/windows/EntityListWindow.h \
           src/gui/windows/EntitySelectWindow.h \
           src/gui/windows/WeatherDialog.h \
           src/io/colors.h \
           src/io/io.h \
           src/io/TempDir.h \
           src/logging/messages.h \
           src/model/Flight.h \
           src/model/Plane.h \
           src/model/Person.h \
           src/model/User.h \
           src/model/LaunchType.h \
           src/model/Entity.h \
           src/plugins/ShellPlugin.h \
           src/statistics/PlaneLog.h \
           src/statistics/PilotLog.h \
           src/time/Date.h \
           src/time/sk_time.h \
           src/time/sk_time_t.h \
           src/time/timeFunctions.h \
           src/web/Argument.h \
		   # Empty line

SOURCES += \
           src/color.cpp \
           src/Condition.cpp \
           src/dataTypes.cpp \
           src/FlightList.cpp \
           src/ObjectField.cpp \
           src/startkladde.cpp \
           src/text.cpp \
           src/version.cpp \
           src/config/Options.cpp \
           src/db/adminFunctions.cpp \
           src/db/DbColumn.cpp \
           src/db/DbEvent.cpp \
           src/db/dbProxy.cpp \
           src/db/DbTable.cpp \
           src/db/dbTypes.cpp \
           src/db/Database.cpp \
           src/documents/LatexDocument.cpp \
           src/documents/Table.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
           src/gui/widgets/AcpiWidget.cpp \
           src/gui/widgets/FlightTable.cpp \
           src/gui/widgets/SkButton.cpp \
           src/gui/widgets/SkComboBox.cpp \
           src/gui/widgets/SkLabel.cpp \
           src/gui/widgets/SkTreeWidgetItem.cpp \
           src/gui/widgets/SkListWidget.cpp \
           src/gui/widgets/SkTable.cpp \
           src/gui/widgets/SkTableItem.cpp \
           src/gui/widgets/SkTextBox.cpp \
           src/gui/widgets/WeatherWidget.cpp \
           src/gui/widgets/LabelComboBox.cpp \
           src/gui/widgets/SkTimeEdit.cpp \
           src/gui/windows/DateWindow.cpp \
           src/gui/windows/FlightWindow.cpp \
           src/gui/windows/MainWindow.cpp \
           src/gui/windows/SkDialog.cpp \
           src/gui/windows/SplashScreen.cpp \
           src/gui/windows/StatisticsWindow.cpp \
           src/gui/windows/EntityEditWindow.cpp \
           src/gui/windows/EntityListWindow.cpp \
           src/gui/windows/EntitySelectWindow.cpp \
           src/gui/windows/WeatherDialog.cpp \
           src/io/io.cpp \
           src/io/TempDir.cpp \
           src/logging/messages.cpp \
           src/model/Flight.cpp \
           src/model/Plane.cpp \
           src/model/Person.cpp \
           src/model/User.cpp \
           src/model/LaunchType.cpp \
           src/model/Entity.cpp \
           src/plugins/DataFormatPlugin.cpp \
           src/plugins/ShellPlugin.cpp \
           src/statistics/PlaneLog.cpp \
           src/statistics/PilotLog.cpp \
           src/time/Date.cpp \
           src/time/sk_time.cpp \
           src/time/sk_time_t.cpp \
           src/time/timeFunctions.cpp \
           src/web/Argument.cpp \
		   # Empty line

