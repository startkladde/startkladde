# This only detects subversion working copies if they contain a .svn directory.
# This seems to be the case for both svn and TortoiseSVN at the moment, but we
# may want to use the subversion client - if found - to determine whether the
# source directory is a working directory.

SET (svn_dir "${SOURCE_DIR}/.svn")

IF(EXISTS "${svn_dir}" AND IS_DIRECTORY "${svn_dir}")
	# Looks like this is an SVN directory. Try to find SVN.
	FIND_PACKAGE (Subversion)
	
	if (SUBVERSION_FOUND)
		# Extract working copy information into SVN_... variables
		Subversion_WC_INFO (${SOURCE_DIR} SVN)
		SET (SVN_REVISION "${SVN_WC_REVISION}")
		MESSAGE (STATUS "SVN working copy, revision is ${SVN_REVISION}")
	else ()
		# Set dummy variables
		SET (SVN_REVISION "?")
		MESSAGE (STATUS "SVN working copy, unknown revision (svn not found)")
	endif ()
ELSE ()
	SET (SVN_REVISION "")
	MESSAGE (STATUS "Not an SVN working copy")
ENDIF ()

# Write the header file contents to a temporary file
file (WRITE svnVersion.h.tmp "#define SVN_VERSION \"${SVN_REVISION}\"\n")

# Copy the file to the final file (only if it changed)
execute_process (COMMAND ${CMAKE_COMMAND} -E copy_if_different svnVersion.h.tmp svnVersion.h)
