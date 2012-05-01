# TODO compiler options: add -Wall -Wextra
# TODO installation (cpack): include required Qt plugins (Windows only), external plugins
# TODO make uninstall (see SpeedCrunch)


#################
## cmake setup ##
#################

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their
# definition and dump it in the cache along with proper documentation, otherwise
# set CMAKE_BUILD_TYPE to Release prior to calling PROJECT()
# This must be performed before PROJECT().
IF(DEFINED CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
	SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

cmake_minimum_required (VERSION 2.8.6) # 2.8.6 required for automoc
PROJECT (startkladde)

# Include additional cmake scripts (e. g. finding the MySQL library)
SET (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")


##############
## Packages ##
##############

SET (MYSQL_DIR "" CACHE PATH "Base directory of the MySQL installation (the directory containing bin/, lib/ and include/, only required if MySQL is not autodetected (typically on Windows)")

# Hack: prevent an error message if not found
SET (RUBY_LIBRARY     "not_required")
SET (RUBY_INCLUDE_DIR "not_required")

# Find packages
FIND_PACKAGE (Qt4       REQUIRED)
FIND_PACKAGE (MySQL 5.0 REQUIRED)
FIND_PACKAGE (Acpi              )
FIND_PACKAGE (Ruby      REQUIRED)

# Setup the Qt library
SET(QT_USE_QTSQL     TRUE)
SET(QT_USE_QTNETWORK TRUE)

set(CMAKE_AUTOMOC TRUE)

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
file (GLOB_RECURSE startkladde_FORMS     RELATIVE ${PROJECT_SOURCE_DIR} src/*.ui    )
file (GLOB_RECURSE startkladde_RESOURCES RELATIVE ${PROJECT_SOURCE_DIR} config/*.qrc)

# Some files cannot be MOCed automatically because the header file name does not
# match the class name (?). MOC them manually.
SET (startkladde_MOC
	src/gui/widgets/AcpiWidget.h
	src/model/objectList/ObjectListModel.h
)

if (${ACPI_FOUND})
	SET (AcpiWidget libacpi)
	SET (libs ${libs} ${ACPI_LIBRARIES})
else ()
	SET (AcpiWidget dummy)
endif ()

# Tweak source lists
LIST (REMOVE_ITEM startkladde_SOURCES src/gui/widgets/AcpiWidget_libacpi.cpp)
LIST (REMOVE_ITEM startkladde_SOURCES src/gui/widgets/AcpiWidget_dummy.cpp)
LIST (APPEND      startkladde_SOURCES src/gui/widgets/AcpiWidget_${AcpiWidget}.cpp)
LIST (APPEND      startkladde_SOURCES build/CurrentSchema.cpp)

ADD_CUSTOM_COMMAND (
	OUTPUT "${PROJECT_SOURCE_DIR}/build/CurrentSchema.cpp"
	MAIN_DEPENDENCY src/db/schema/CurrentSchema.cpp.erb
	DEPENDS src/db/migrations/current_schema.yaml
	COMMAND "${RUBY_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/script/build/generate_current_schema_cpp.rb"
)

# Handle Qt sources
QT4_WRAP_CPP      (startkladde_HEADERS_MOC   ${startkladde_MOC}      )
QT4_WRAP_UI       (startkladde_FORMS_HEADERS ${startkladde_FORMS}    )
QT4_ADD_RESOURCES (startkladde_RESOURCES_RCC ${startkladde_RESOURCES})


INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" "${MYSQL_INCLUDE_DIR}")


if(WIN32)
	# TODO do this on or after build, not every time we run cmake.
	# add_custom_command with add_custom_target may or may not be appropriate. 
	# TODO need to check if Qt found first (like MySQL below)?
	# TODO Qt MySQL plugin
	SET (qtlibs QtCore4.dll QtGui4.dll QtSql4.dll QtNetwork4.dll mingwm10.dll libgcc_s_dw2-1.dll)
	foreach (qtlib ${qtlibs})
		#message ("Pulling in Qt library ${qtlib}")
		file (COPY ${QT_BINARY_DIR}/${qtlib} DESTINATION ${PROJECT_BINARY_DIR})
	endforeach()
	SET (INSTALL_LIBS ${INSTALL_LIBS} ${qtlibs})

	# TODO the MySQL finder should export MYSQL_LIBRARY_DIR or MYSQL_BINARY_DIR
	#message ("Pulling in MySQL library libmysql.dll")
	# TODO this should not even be executed if MySQL is not found (?)
	if (MYSQL_INCLUDE_DIR)
		file (COPY ${MYSQL_INCLUDE_DIR}/../lib/libmysql.dll DESTINATION ${PROJECT_BINARY_DIR})
	endif ()
	SET (INSTALL_LIBS ${INSTALL_LIBS} libmysql.dll)
	
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
# Note that QT4_CREATE_TRANSLATION will delete the translation sources (*.ts)
# on "make clean".
QT4_ADD_TRANSLATION(translation_files ${translation_sources})


################
## Executable ##
################

ADD_EXECUTABLE(startkladde
	WIN32                            # GUI application on Windows - ignored on other platforms
	${startkladde_SOURCES} 
	${startkladde_MOC} 
    ${startkladde_FORMS_HEADERS} 
    ${startkladde_RESOURCES_RCC}
    ${startkladde_WINDOWS_RESOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/svnVersion.h
	${translation_files}
)

TARGET_LINK_LIBRARIES (startkladde ${QT_LIBRARIES} ${libs})


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


#############
## Install ##
#############

# Install path definitions
# On Windows, everything is installed into the application directory. On other
# systems, different files are installed into different system directories.
IF (WIN32)
	SET (EXECUTABLE_INSTALL_DIR   ".")
	SET (TRANSLATIONS_INSTALL_DIR ".")
ELSE ()
	SET (EXECUTABLE_INSTALL_DIR   "bin"               )
	SET (TRANSLATIONS_INSTALL_DIR "share/startkladde" )
	SET (MENU_INSTALL_DIR         "share/applications")
	SET (ICON_INSTALL_DIR         "share/pixmaps"     )
ENDIF ()

# Install the executable
install (TARGETS   startkladde                        DESTINATION ${EXECUTABLE_INSTALL_DIR}  )
# Install the translations
install (DIRECTORY ${PROJECT_BINARY_DIR}/translations DESTINATION ${TRANSLATIONS_INSTALL_DIR})
# TODO install the external plugins, but without the SVN directory

# On Windows systems, install the required libraries
IF (WIN32)
	foreach (lib ${INSTALL_LIBS})
		install (FILES ${PROJECT_BINARY_DIR}/${lib} DESTINATION ${EXECUTABLE_INSTALL_DIR})
	endforeach ()
ENDIF ()
# TODO install the Qt translations

# On non-Windows systems, install the Freedesktop menu and icon
#IF (NOT WIN32)
#	INSTALL (FILES startkladde.desktop       DESTINATION ${MENU_INSTALL_DIR})
#	INSTALL (FILES graphics/startkladde.png DESTINATION ${ICON_INSTALL_DIR})
#ENDIF ()

###############
## Installer ##
###############

#include (InstallRequiredSystemLibraries)
#set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${startkladde_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${startkladde_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${startkladde_VERSION_REVISION}")

# TODO investigate cpack options:
#INCLUDE(InstallRequiredSystemLibraries)
#SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A flight logging software for small airfields")
#SET(CPACK_PACKAGE_VENDOR "Startkladde team")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "startkladde ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
#SET(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")
#SET(CPACK_SOURCE_PACKAGE_FILE_NAME "startkladde-${VERSION}${VERSION_SUFFIX}")
#SET(CPACK_SOURCE_GENERATOR TBZ2)
#SET(CPACK_SYSTEM_NAME "")
#SET(CPACK_TOPLEVEL_TAG "")
#SET(CPACK_SOURCE_IGNORE_FILES CMakeLists.txt.user "~$" ".svn$")
#
#IF (WIN32)
#	SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
#	SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
#	SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
#	SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
#	SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
#	SET(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
#	SET(CPACK_NSIS_MODIFY_PATH ON)
#ELSE ()
#	SET(CPACK_STRIP_FILES "bin/MyExecutable")
#	SET(CPACK_SOURCE_STRIP_FILES "")
#ENDIF ()

include (CPack)

