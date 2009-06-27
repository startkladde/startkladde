# 0wxxxxifind . -name A€kb€kb!!sh0xxi           A \j
CONFIG += qt debug
QT += qt3support
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi
MAKEFILE = Makefile_sk_admin
OBJECTS_DIR = build/
MOC_DIR= build/

# Input
HEADERS += \
           src/Condition.h \
           src/dataTypes.h \
           src/text.h \
           src/version.h \
           src/config/Options.h \
           src/db/Database.h \
           src/db/DbColumn.h \
           src/db/DbTable.h \
           src/db/adminFunctions.h \
           src/db/dbProxy.h \
           src/db/dbTypes.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/io/io.h \
           src/logging/messages.h \
           src/model/Entity.h \
           src/model/Flight.h \
           src/model/LaunchType.h \
           src/model/Person.h \
           src/model/Plane.h \
           src/model/User.h \
           src/plugins/ShellPlugin.h \
           src/plugins/DataFormatPlugin.h \
           src/time/Date.h \
           src/time/sk_time_t.h \
           src/time/timeFunctions.h \
           src/web/argument.h \
		   # Empty line

SOURCES += \
           src/Condition.cpp \
           src/dataTypes.cpp \
           src/sk_admin.cpp \
           src/text.cpp \
           src/version.cpp \
           src/config/Options.cpp \
           src/db/adminFunctions.cpp \
           src/db/DbColumn.cpp \
           src/db/dbProxy.cpp \
           src/db/DbTable.cpp \
           src/db/dbTypes.cpp \
           src/db/Database.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
           src/io/io.cpp \
           src/logging/messages.cpp \
           src/model/Flight.cpp \
           src/model/Plane.cpp \
           src/model/Person.cpp \
           src/model/User.cpp \
           src/model/LaunchType.cpp \
           src/model/Entity.cpp \
           src/plugins/DataFormatPlugin.cpp \
           src/plugins/ShellPlugin.cpp \
           src/time/Date.cpp \
           src/time/sk_time_t.cpp \
           src/time/timeFunctions.cpp \
           src/web/argument.cpp \
		   # Empty line


