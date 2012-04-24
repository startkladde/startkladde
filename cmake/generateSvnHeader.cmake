include (FindSubversion)

# Extract working copy information into SVN_... variables
Subversion_WC_INFO (${SOURCE_DIR} SVN)

# Write the header file contents to a temporary file
file (WRITE svnVersion.h.txt "#define SVN_VERSION \"${SVN_WC_REVISION}\"\n")

# Copy the file to the final file (only if it changed)
execute_process (COMMAND ${CMAKE_COMMAND} -E copy_if_different svnVersion.h.txt svnVersion.h)
                        
                        
# From FindSubversion.cmake:                        
#  FIND_PACKAGE(Subversion)
#  IF(SUBVERSION_FOUND)
#    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#    Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
#  ENDIF(SUBVERSION_FOUND)
