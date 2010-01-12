CONFIG += qt debug
QT += qt3support
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi
MAKEFILE = Makefile_sk_web
OBJECTS_DIR = build/
MOC_DIR= build/
QMAKE_CXXFLAGS += -Werror

# Input
HEADERS += \
           src/Condition.h \
           src/dataTypes.h \
           src/ObjectField.h \
           src/text.h \
           src/version.h \
           src/config/options.h \
           src/db/DbColumn.h \
           src/db/dbProxy.h \
           src/db/DbTable.h \
           src/db/dbTypes.h \
           src/db/Database.h \
           src/documents/LatexDocument.h \
           src/documents/Table.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/io/io.h \
           src/io/TempDir.h \
           src/logging/messages.h \
           src/model/Flight.h \
           src/model/Plane.h \
           src/model/Person.h \
           src/model/User.h \
           src/model/LaunchType.h \
           src/model/Entity.h \
           src/plugins/DataFormatPlugin.h \
           src/plugins/ShellPlugin.h \
           src/statistics/PlaneLog.h \
           src/statistics/PilotLog.h \
           src/time/Date.h \
           src/time/Time.h \
           src/time/timeFunctions.h \
           src/web/Argument.h \
           src/web/HtmlDocument.h \
           src/web/HttpDocument.h \
           src/web/MimeHeader.h \
           src/web/WebSession.h \
           src/web/WhatNext.h \
		   # Empty line

SOURCES += \
           src/Condition.cpp \
           src/dataTypes.cpp \
           src/ObjectField.cpp \
           src/sk_web.cpp \
           src/text.cpp \
           src/version.cpp \
           src/config/options.cpp \
           src/db/DbColumn.cpp \
           src/db/dbProxy.cpp \
           src/db/DbTable.cpp \
           src/db/dbTypes.cpp \
           src/db/Database.cpp \
           src/documents/LatexDocument.cpp \
           src/documents/Table.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
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
           src/time/Time.cpp \
           src/time/timeFunctions.cpp \
           src/web/Argument.cpp \
           src/web/HtmlDocument.cpp \
           src/web/HttpDocument.cpp \
           src/web/MimeHeader.cpp \
           src/web/WebSession.cpp \
           src/web/WhatNext.cpp \
		   # Empty line

