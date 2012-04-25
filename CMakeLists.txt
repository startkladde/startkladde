# TODO handle: svn not found, not a working copy
# TODO SK_WINDOWS on Windows, or individually configure the values it is used
# for
# TODO use RELEASE as default build type?
# TODO the following compiler options and defines were used by qmake:
#   -Wextra -O2 -frtti -fexceptions -mthreads -Wall
#   -DQT_LARGEFILE_SUPPORT -DQT_NEEDS_QMAIN -DQT_THREAD_SUPPORT -DUNICODE
#   -DQT_HAVE_MMX -DQT_HAVE_3DNOW -DQT_HAVE_SSE -DQT_HAVE_MMXEXT -DQT_HAVE_SSE2
# TODO clean up compiler options, add -Wall
# TODO installation (cpack): include required Qt plugins (Windows only) and translations
# TODO remove HEADERS after using AUTOMOC
# TODO does make clean remove the translation sources?


#################
## cmake setup ##
#################

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

# Setup the Qt library
SET(QT_USE_QTSQL     TRUE)
SET(QT_USE_QTNETWORK TRUE)

# This is available starting 2.8.6 which is not currently available in the
# latest Ubuntu release.
#set(CMAKE_AUTOMOC TRUE)

# For gcc and release mode, enable some options that make the binary smaller
if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        -DQT_NO_DEBUG") # Disable QT debug code
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s"           ) # Strip binary
endif ()





##################
## Source files ##
##################

# Make lists of source files
file (GLOB_RECURSE startkladde_SOURCES   RELATIVE ${PROJECT_SOURCE_DIR} src/*.cpp   )
file (GLOB_RECURSE startkladde_HEADERS   RELATIVE ${PROJECT_SOURCE_DIR} src/*.h     )
file (GLOB_RECURSE startkladde_FORMS     RELATIVE ${PROJECT_SOURCE_DIR} src/*.ui    )
file (GLOB_RECURSE startkladde_RESOURCSE RELATIVE ${PROJECT_SOURCE_DIR} config/*.qrc)

# Tweak source lists
LIST (REMOVE_ITEM startkladde_SOURCES src/gui/widgets/AcpiWidget_libacpi.cpp)
LIST (APPEND      startkladde_SOURCES build/CurrentSchema.cpp)

# Handle Qt sources
QT4_WRAP_CPP      (startkladde_HEADERS_MOC   ${startkladde_HEADERS}  )
QT4_WRAP_UI       (startkladde_FORMS_HEADERS ${startkladde_FORMS}    )
QT4_ADD_RESOURCES (startkladde_RESOURCES_RCC ${startkladde_RESOURCES})


INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" "${MYSQL_INCLUDE_DIR}")


if(WIN32)
	# TODO do this on or after build, not every time we run cmake.
	# add_custom_command with add_custom_target may or may not be appropriate. 
	SET (qtlibs QtCore4.dll QtGui4.dll QtSql4.dll QtNetwork4.dll mingwm10.dll libgcc_s_dw2-1.dll)
	foreach (qtlib ${qtlibs})
		#message ("Pulling in Qt library ${qtlib}")
		file (COPY ${QT_BINARY_DIR}/${qtlib} DESTINATION ${PROJECT_BINARY_DIR})
	endforeach()

	# TODO the MySQL finder should export MYSQL_LIBRARY_DIR or MYSQL_BINARY_DIR
	#message ("Pulling in MySQL library libmysql.dll")
	file (COPY ${MYSQL_INCLUDE_DIR}/../lib/libmysql.dll DESTINATION ${PROJECT_BINARY_DIR})
	
	SET (startkladde_WINDOWS_RESOURCES config/startkladde.rc)
endif()

set (startkladde_VERSION_MAJOR 2)
set (startkladde_VERSION_MINOR 1)
set (startkladde_VERSION_REVISION 0)

configure_file ("${PROJECT_SOURCE_DIR}/src/build.h.in" "${PROJECT_BINARY_DIR}/build.h")


##################
## Translations ##
##################

# Make a list of translation source files (absolute paths)
file (GLOB translation_sources translations/*.ts)

# Create the translations/ directory in the build directory
file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/translations)

# Set the OUTPUT_LOCATION of the translation source files to the translations/
# directory in the build directory. QT4_CREATE_TRANSLATION will place the
# compiled translations there.
SET_SOURCE_FILES_PROPERTIES (${translation_sources} PROPERTIES OUTPUT_LOCATION ${PROJECT_BINARY_DIR}/translations)

# Add rules for compiling the translation files
QT4_CREATE_TRANSLATION(translation_files ${translation_sources})


################
## Executable ##
################

ADD_EXECUTABLE(startkladde
	WIN32                            # GUI application on Windows - ignored on other platforms
	${startkladde_SOURCES} 
	${startkladde_HEADERS_MOC} 
    ${startkladde_FORMS_HEADERS} 
    ${startkladde_RESOURCES_RCC}
    ${startkladde_WINDOWS_RESOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
     ${translation_files}
)

TARGET_LINK_LIBRARIES (startkladde ${QT_LIBRARIES})


#########
## SVN ##
#########

# See http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision

ADD_CUSTOM_TARGET (svnheader ALL)

# Create svnVersion.h using cmake script
ADD_CUSTOM_COMMAND(
	TARGET svnheader
	COMMAND ${CMAKE_COMMAND}
		-DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} 
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generateSvnHeader.cmake
)

# Mark svnVersion.h as generated
set_source_files_properties (
	${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

# The executable depends on the svnheader target
add_dependencies (startkladde svnheader)
