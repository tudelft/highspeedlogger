cmake_minimum_required (VERSION 2.6) 

project ( highspeedlogger ) 

if(WIN32)
   if(MSVC)
      add_definitions(-DWIN32_LEAN_AND_MEAN)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
   endif(MSVC)
else()
   add_definitions(-Wno-write-strings)
endif(WIN32)

set ( EXECUTABLE_OUTPUT_PATH "${highspeedlogger_SOURCE_DIR}/bin" )
# set ( CMAKE_CFG_INTDIR "" )


add_subdirectory (download)
add_subdirectory (pprz_download)
add_subdirectory (convert)
add_subdirectory (format)
#add_subdirectory (xsens)

