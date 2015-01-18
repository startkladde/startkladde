**Compiling**
=============

This page describes the way to compile Startkladde from the source code
on Windows and Linux. These instructions only apply to the user program,
not the web interface (sk\_web), which is not compiled.

This page is intended for developers. If you only want to use the
program, you can download it from the [download
page](http://startkladde.sourceforge.net/download.html).

To compile the program, 4 steps are required:

-   Step 1: get the source code

-   Step 2: install required software

-   Step 3: configure the build

-   Step 4: compile the program

Each of the steps is described in more detail below.

There are many different ways to compile the program. The following ways
have been tested and are described on this page:

-   using the command line - the fastest way

-   using the Eclipse IDE

-   using the Qt Creator IDE

It is also possible to use Microsoft Visual C++. However, because
of [some problems with the Visual C++
compiler](http://medini.org/software/msviscxx/msviscxx.html), this may
require changes to the source code.

**Quick start (Linux command line)**
------------------------------------

If you're using the command line on Ubuntu Linux, the following steps
can be used to compile the current SVN trunk version of the program. For
other operating systems, other versions or more detailed instructions,
read on.

sudo aptitude install subversion g++ cmake make ruby libqt4-dev
libqt4-sql-mysql libmysqlclient-dev libacpi-dev libudev-dev

svn checkout
http:*//svn.code.sf.net/p/startkladde/code/trunk/startkladde*

mkdir startkladde\_build

cd startkladde\_build

cmake ../startkladde

make

**Step 1: get the source code**
-------------------------------

The source code can be retrieved from the [download
page](https://sourceforge.net/projects/startkladde/files) or from
the [SVN repository](https://sourceforge.net/p/startkladde/wiki/SVN).

Download and/or extract the source code. We'll call the directory where
the source code is located the source directory. Specifically, the
source directory is the directory containing the CMakeLists.txt file. On
Windows, the directory should not contain any spaces.

**Step 2: install required software**
-------------------------------------

### **Linux**

The required software for compiling Startkladde is available as packages
for the major Linux distributions. The following software must be
installed:

  ------------------------------ ------------------------- ---------------------------- ---------------------------- -------------------------------------
  **Software**                   **Ubuntu package name**   **OpenSUSE package name**    **Fedora package name**      **Notes**
  C++ compiler                   g++                       g++                          gcc-c++                      
  make                           make                      FIXME                        FIXME                        
  CMake                          cmake                     cmake                        cmake                        Version 2.8.6 or higher is required
  CMake GUI                      cmake-qt-gui              cmake-gui                    cmake-gui                    
  Ruby interpreter               ruby                      ruby                         ruby                         
  Qt 4 library (with headers)    libqt4-dev                libqt4-devel                 qt-devel                     Version 4.8.0 or higher is required
  Qt 4 MySQL plugin              libqt4-sql-mysql          libqt4-sql-mysql             qt-mysql                     
  MySQL library (with headers)   libmysqlclient-dev        libmysqlclient-devel         mysql-devel                  
  ACPI library (with headers)    libacpi-dev               (not available, see below)   (not available, see below)   
  UDEV library (with headers     libudev-dev               ?                            ?                            
  ------------------------------ ------------------------- ---------------------------- ---------------------------- -------------------------------------

The ACPI library, libacpi-dev is not required. However, without the ACPI
library, battery state display will not be available. On distributions
where the ACPI library is not available, it can be downloaded
from <http://ngolde.de/libacpi.html.> Use make to compile it and make
install (as root) to install it.

### **Windows**

You will have to install a compiler and the Qt libraries. Probably the
easiest way to do this on Windows is to use the Qt SQK which contains
the MinGW compiler, and everything related to the Qt libraries.

The following software must be installed:

-   [CMake](http://cmake.org/cmake/resources/software.html) (version
    > 2.8.6 or higher)

-   [Qt SDK for Windows](http://qt.nokia.com/products/qt-sdk) (during
    > installation, make sure that the source code is installed)

-   [MySQL Server](http://www.mysql.com/downloads), choose "MySQL
    > Installer" (during installation, make sure that "Client C API
    > library" is installed). You will need this even if you are not
    > going to use the MySQL server on this machine because it includes
    > the client library.

-   [Ruby](http://rubyinstaller.org/) (during installation, make sure
    > that "Add Ruby executables to your PATH" is enabled)

You also need the following packages from Microsoft. Depending on the
software installed on your system, you mal already have these packages.

-   [Visual C++ 2008 redistributable
    > package](http://www.microsoft.com/en-us/download/details.aspx?id=29)

-   [Visual C++ 2010 redistributable
    > package](http://www.microsoft.com/en-us/download/details.aspx?id=5555)

-   [.NET Framework
    > 4](http://www.microsoft.com/en-us/download/details.aspx?id=17851)

You must now add the compiler and the Qt build utilities to the path.
Open Control panel -\> System -\> Advanced -\> Environment variables.
Under "System Variables", edit the entry Path and add the following
directories, separated by a semicolon:

-   the mingw\\bin directory in the Qt SDK installation

-   the Desktop\\Qt\\x.y.z\\mingw\\bin directory in the Qt SDK
    > installation, where x.y.z is the version to use.

For example, if you installed the Qt SDK 4.8.1 to the default location,
c:\\QtSDK, the following paths must be added:

-   c:\\QtSDK\\mingw\\bin

-   c:\\QtSDK\\Desktop\\Qt\\4.8.1\\mingw\\bin

To verify this, you can open a command prompt and
execute mingw32-make --version and qmake --version. This should print
the version of the respective utilities.

On Windows, you have to compile the [Qt MySQL
plugin](https://sourceforge.net/p/startkladde/wiki/MysqlPlugin).

**Step 3: configure the build**
-------------------------------

The program is built in a directory separate from the source directory
("out-of-source build"). We'll call this directory the ''build
directory''. The build directory can be located anywhere in the file
system, but should not be a subdirectory of the source directory. If you
are unsure, use a directory calledstartkladde\_build in the same parent
directory as the source directory.

We'll use the CMake GUI for invoking cmake. Open the CMake GUI and enter
the source and build directories in the respective input fields (or use
the\
respective "Browse" buttons). Press the "Configure" button. If build
directory does not exist, it will be created now.

You will be asked to choose a "generator". The choice depends on how you
are going to compile the program:

-   if you are going to use the command line, choose "Unix Makefiles"
    > (Linux) or "MinGW Makefiles" (Windows)

-   if you are going to use Eclipse, choose "Eclipse CDT4 - Unix
    > Makefiles" (Linux) or "Eclipse CDT4 - MinGW Makefiles" (Windows)

-   if you are going to use Qt Creator, choose "CodeBlocks - Unix
    > Makefiles" (Linux) or "CodeBlocks - MinGW Makefiles" (Windows)

In any case, specify "Use default native compilers". Press "Finish".
CMake will perform various checks and try to find the required
libraries. A number of configuration options will appear in the list in
the center of the window. These entries may be colored red - this does
not indicate an error; the CMake GUI colors all newly discovered options
red.

On Windows, CMake will probably report an error because it was unable to
locate the MySQL library. Make sure that the option "Grouped" right
above the list is enabled. In the list in the center of the window, open
the "MYSQL" section. Click on the "MYSQL\_DIR" row and use the brows
button (labeled "...") at the right of that row to locate the MySQL
server directory. Specifically, choose the directory from the MySQL
installation that contains the "include" and "lib" directores. Then,
press "Configure" again and the configuration should finish without
further errors.

When configuration succeeds, "Configuring done" will appear in the text
box at the bottom. Press "Generate" and "Generating done" should appear
in the text box.

If you want to change the generator to use a different method for
compiling, select "Delete Cache" from the "File" menu and then press
"Configure" again. You will have to repeat the above procedure.

Configuration only has to be performed once. It does not have to be
repeated each time source code changes.

**Step 4: compile the program**
-------------------------------

Use one of the methods below, depending on which generator you selected
in the previous step ("Configuring the build").

### **Using the command line**

-   open a terminal/command prompt

-   change into the build directory

-   call make (Linux) or mingw32-make (Windows)

This should compile the program. After the compilition has finished, you
can run the program by calling ./startkladde (Linux) or startkladde on
Windows, or by running the program from the GUI.

### **Using Eclipse**

You will need to install the Eclipse IDE with the C/C++ development
tools (CDT). The easiest way to achieve this is to install "Eclipse for
C/C++\
developers" from the [Eclipse download
website](http://eclipse.org/downloads). On Linux, it may also be
possible to use a package.

In Eclipse, select "Import" from the "File" menu. From the "General"
category, choose "Existing Projects into Workspace". Check "Select root
directory" and enter the build directory (or use the "Browse" button).
In the "Projects:" list, the project will appear. Make sure it is
selected and the option "Copy projects into workspace" is deselected.
Click "Finish". This will add the project to the list.

You can now use Project - Build All (or Ctrl+B) to compile the project.

To run the program, right-click the project in the "Project Explorer"
list and choose Run as - Local C/C++ application (this can also be found
in the Runmenu). Select the generated executable (startkladde on
Linux, startkladde.exe on Windows). If you are asked for a "debug
configuration", if unsure, choose "gdb/mi".

On subsequent runs, you can just use Run - Run (or Ctrl+F11). The
program will be compiled automatically.

### **Using Qt Creator**

The Qt Creator is installed with the Qt SDK, so you probably have it.

In Qt Creator, on the "Welcome" page (selected on the left of the
window), choose "Open project". Open the file CMakeLists from the source
directory. You wil be asked for the build path. Enter or browse to the
build directory. On the next page, press "Finish" (there is no need to
run CMake again). The project will open.

You will have to tell Qt Creator to use mingw32-make instead of make. On
the "Projects" page (selected on the left of the window), open "Build
settings" (selected on the top of the window). Under "Build steps",
remove the entry "Unknown toolchain". Add a user defined build step,
specifying mingw32-make as the command. Leave the working directory
at %{buildDir}.

You can now use Build - Build all (or Ctrl+B, or the buttons at the
lower left of the window) to compile the project.

To run the program, use Create - Run (or Ctrl+R, or the buttons on the
lower left of the window). When asked for an executable,
select startkladde.exe.

On subsequent runs, you can just use Run (or Ctrl+R). The program will
be compiled automatically.

Qt Creator creates a file CMakeLists.txt.user in the source directory,
containing project settings. If you want to change the build directory,
delete this file.

**Making changes**
------------------

Generally, after making a change, only the last build step has to be
repeated. You do not have to configure the build repeatedly.

When making certain changes, such as adding or renaming a file, it may
be necessary to re-save (without making any changes) CMakeLists.txt (or
executing touch CMakeLists.txt). The reason is that CMake stores a list
of files to compile in a cache to speed up the build. Re-saving the file
forces the cache to be regenerated.

