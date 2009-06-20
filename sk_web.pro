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
           src/config/options.h \
           src/data_types.h \
           src/db/db_column.h \
           src/db/db_proxy.h \
           src/db/db_table.h \
           src/db/db_types.h \
           src/db/sk_db.h \
           src/documents/latex_document.h \
           src/documents/table.h \
           src/flight_list.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/io/io.h \
           src/io/temp_dir.h \
           src/logging/messages.h \
           src/model/sk_flug.h \
           src/model/sk_flugzeug.h \
           src/model/sk_person.h \
           src/model/sk_user.h \
           src/model/startart_t.h \
           src/model/stuff.h \
           src/object_field.h \
           src/plugins/plugin_data_format.h \
           src/plugins/sk_plugin.h \
           src/statistics/bordbuch.h \
           src/statistics/flugbuch.h \
           src/text.h \
           src/time/sk_date.h \
           src/time/sk_time_t.h \
           src/time/time_functions.h \
           src/version.h \
           src/version/version.h \
           src/web/argument.h \
           src/web/html_document.h \
           src/web/http_document.h \
           src/web/mime_header.h \
           src/web/web_session.h \
           src/web/what_next.h \

SOURCES += \
           src/condition_t.cpp \
           src/config/options.cpp \
           src/data_types.cpp \
           src/db/db_column.cpp \
           src/db/db_proxy.cpp \
           src/db/db_table.cpp \
           src/db/db_types.cpp \
           src/db/sk_db.cpp \
           src/documents/latex_document.cpp \
           src/documents/table.cpp \
           src/flight_list.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
           src/io/io.cpp \
           src/io/temp_dir.cpp \
           src/logging/messages.cpp \
           src/model/sk_flug.cpp \
           src/model/sk_flugzeug.cpp \
           src/model/sk_person.cpp \
           src/model/sk_user.cpp \
           src/model/startart_t.cpp \
           src/model/stuff.cpp \
           src/object_field.cpp \
           src/plugins/plugin_data_format.cpp \
           src/plugins/sk_plugin.cpp \
           src/sk_web.cpp \
           src/statistics/bordbuch.cpp \
           src/statistics/flugbuch.cpp \
           src/text.cpp \
           src/time/sk_date.cpp \
           src/time/sk_time_t.cpp \
           src/time/time_functions.cpp \
           src/version.cpp \
           src/web/argument.cpp \
           src/web/html_document.cpp \
           src/web/http_document.cpp \
           src/web/mime_header.cpp \
           src/web/web_session.cpp \
           src/web/what_next.cpp \

