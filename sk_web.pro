# 0wxxxxifind . -name Akbkb!!sh0xxi           A \j
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

# Input
HEADERS += \
           src/condition_t.h \
           src/data_types.h \
           src/flight_list.h \
           src/object_field.h \
           src/text.h \
           src/version.h \
           src/config/options.h \
           src/db/db_column.h \
           src/db/db_proxy.h \
           src/db/db_table.h \
           src/db/db_types.h \
           src/db/sk_db.h \
           src/documents/latex_document.h \
           src/documents/table.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/io/io.h \
           src/io/temp_dir.h \
           src/logging/messages.h \
           src/model/Flight.h \
           src/model/Plane.h \
           src/model/Person.h \
           src/model/User.h \
           src/model/LaunchType.h \
           src/model/Entity.h \
           src/plugins/plugin_data_format.h \
           src/plugins/sk_plugin.h \
           src/statistics/bordbuch.h \
           src/statistics/flugbuch.h \
           src/time/sk_date.h \
           src/time/sk_time_t.h \
           src/time/time_functions.h \
           src/web/argument.h \
           src/web/html_document.h \
           src/web/http_document.h \
           src/web/mime_header.h \
           src/web/web_session.h \
           src/web/what_next.h \
		   # Empty line

SOURCES += \
           src/condition_t.cpp \
           src/data_types.cpp \
           src/flight_list.cpp \
           src/object_field.cpp \
           src/sk_web.cpp \
           src/text.cpp \
           src/version.cpp \
           src/config/options.cpp \
           src/db/db_column.cpp \
           src/db/db_proxy.cpp \
           src/db/db_table.cpp \
           src/db/db_types.cpp \
           src/db/sk_db.cpp \
           src/documents/latex_document.cpp \
           src/documents/table.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
           src/io/io.cpp \
           src/io/temp_dir.cpp \
           src/logging/messages.cpp \
           src/model/Flight.cpp \
           src/model/Plane.cpp \
           src/model/Person.cpp \
           src/model/User.cpp \
           src/model/LaunchType.cpp \
           src/model/Entity.cpp \
           src/plugins/plugin_data_format.cpp \
           src/plugins/sk_plugin.cpp \
           src/statistics/bordbuch.cpp \
           src/statistics/flugbuch.cpp \
           src/time/sk_date.cpp \
           src/time/sk_time_t.cpp \
           src/time/time_functions.cpp \
           src/web/argument.cpp \
           src/web/html_document.cpp \
           src/web/http_document.cpp \
           src/web/mime_header.cpp \
           src/web/web_session.cpp \
           src/web/what_next.cpp \
		   # Empty line

