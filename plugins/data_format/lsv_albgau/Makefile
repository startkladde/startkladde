
# Diese beiden Variablen müssen auf die richten Verzeichnisse eingestellt sein
# oder hier angepasst werden.
#QTDIR    ?= /usr/share/qt3
QTDIR    ?= /usr/share/qt4
#MYSQLDIR ?= /usr/local/mysql

# Für abweichende Verzeichnisstrukturen können die folgenden Verzeichnisse
# explizit angegeben werden.
QT_INCLUDE_DIR    ?= $(QTDIR)/include
QT_LIB_DIR        ?= $(QTDIR)/lib
#MYSQL_INCLUDE_DIR ?= $(MYSQLDIR)/include
#MYSQL_LIB_DIR     ?= $(MYSQLDIR)/lib
MYSQL_INCLUDE_DIR ?= /usr/include/mysql
MYSQL_LIB_DIR     ?= /usr/lib/mysql

SKDIR=../../..

# QT3:
#QT_INCLUDE   = -I$(QT_INCLUDE_DIR)
#LIB=-lqt-mt
# QT4:
QT_INCLUDE   = -I$(QT_INCLUDE_DIR) -I$(QT_INCLUDE_DIR)/Qt
LIB=-lQtGui -lQtCore -lQt3Support

CPPFLAGS=-DQT3_SUPPORT

all: format_lsv_albgau.so

format_lsv_albgau.o: format_lsv_albgau.cpp	\
	$(SKDIR)/src/web/argument.h \
	$(SKDIR)/src/object_field.h \
	$(SKDIR)/src/model/sk_flug.h	\
	$(SKDIR)/src/model/sk_flugzeug.h \
	$(SKDIR)/src/model/sk_person.h \
	$(SKDIR)/src/model/startart_t.h	\
	$(SKDIR)/src/plugins/plugin_data_format.h
	gcc -I$(SKDIR) $(QT_INCLUDE) -I$(MYSQL_INCLUDE_DIR) -fPIC -Wall -c format_lsv_albgau.cpp

format_lsv_albgau.so: format_lsv_albgau.o
	gcc -shared -o format_lsv_albgau.so format_lsv_albgau.o

.PHONY: clean
clean:
	-rm -f *~ *.o *.so *.ii *.s 2>/dev/null 

