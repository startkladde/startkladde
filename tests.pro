CONFIG += qt debug
QT += qt3support
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi -lcppunit
MAKEFILE = Makefile_tests
OBJECTS_DIR = build/
MOC_DIR= build/
QMAKE_CXXFLAGS += -Werror -DCPPUNIT_MAIN=main

# Input
HEADERS += \
           test/TestRunnerClient.h \
           src/text.h \
           src/db/dbTypes.h \
           src/io/colors.h \
           src/logging/messages.h \
		   test/textTest.h \
		   # Empty line

SOURCES += \
           test/TestRunnerClient.cpp \
           src/text.cpp \
           src/db/dbTypes.cpp \
		   src/logging/messages.cpp \
		   test/textTest.cpp \
		   # Empty line


