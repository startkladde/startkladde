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
HEADERS += src/argument.h \
           src/bordbuch.h \
           src/condition_t.h \
           src/data_types.h \
           src/db_column.h \
           src/db_proxy.h \
           src/db_table.h \
           src/db_types.h \
           src/dialogs.h \
           src/flight_list.h \
           src/flugbuch.h \
           src/html_document.h \
           src/http_document.h \
           src/io.h \
           src/latex_document.h \
           src/messages.h \
           src/mime_header.h \
           src/object_field.h \
           src/options.h \
           src/plugin_data_format.h \
           src/settings.h \
           src/sk_date.h \
           src/sk_db.h \
           src/sk_flug.h \
           src/sk_flugzeug.h \
           src/sk_person.h \
           src/sk_plugin.h \
           src/sk_time_t.h \
           src/sk_user.h \
           src/startart_t.h \
           src/stuff.h \
           src/table.h \
           src/temp_dir.h \
           src/text.h \
           src/time_functions.h \
           src/version.h \
           version/version.h \
           src/web_session.h \
           src/what_next.h
SOURCES += src/argument.cpp \
           src/bordbuch.cpp \
           src/condition_t.cpp \
           src/data_types.cpp \
           src/db_column.cpp \
           src/db_proxy.cpp \
           src/db_table.cpp \
           src/db_types.cpp \
           src/dialogs.cpp \
           src/flight_list.cpp \
           src/flugbuch.cpp \
           src/html_document.cpp \
           src/http_document.cpp \
           src/io.cpp \
           src/latex_document.cpp \
           src/messages.cpp \
           src/mime_header.cpp \
           src/object_field.cpp \
           src/options.cpp \
           src/plugin_data_format.cpp \
           src/settings.cpp \
           src/sk_date.cpp \
           src/sk_db.cpp \
           src/sk_flug.cpp \
           src/sk_flugzeug.cpp \
           src/sk_person.cpp \
           src/sk_plugin.cpp \
           src/sk_time_t.cpp \
           src/sk_user.cpp \
		   src/sk_web.cpp \
           src/startart_t.cpp \
           src/stuff.cpp \
           src/table.cpp \
           src/temp_dir.cpp \
           src/text.cpp \
           src/time_functions.cpp \
           src/version.cpp \
           src/web_session.cpp \
           src/what_next.cpp

