cmake_minimum_required (VERSION 2.6)

# Compiler Flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DJSON_DEBUG")
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Require global XMM project
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/_build_tmp)

# PYTHON STUFF
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
execute_process( COMMAND python -c "from distutils import sysconfig; print sysconfig.PREFIX" OUTPUT_VARIABLE PYTHON_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND python -c "import sys; a = sys.version; print a[:3]" OUTPUT_VARIABLE PYVERSION OUTPUT_STRIP_TRAILING_WHITESPACE )
if(PYTHON_PREFIX)
    if(EXISTS "${PYTHON_PREFIX}/lib/libpython${PYVERSION}.dylib")
        set(PYTHON_LIBRARY "${PYTHON_PREFIX}/lib/libpython${PYVERSION}.dylib")
        set(PYTHON_INCLUDE_DIR "${PYTHON_PREFIX}/Headers")
    endif(EXISTS "${PYTHON_PREFIX}/lib/libpython${PYVERSION}.dylib")
endif(PYTHON_PREFIX)
find_package(PythonLibs)
message(STATUS "PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_PATH: ${PYTHON_INCLUDE_PATH}")
message(STATUS "PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHONLIBS_VERSION_STRING: ${PYTHONLIBS_VERSION_STRING}")

## Try to determine the directory of numpy headers
execute_process( COMMAND python -c "from numpy import distutils; path=distutils.misc_util.get_numpy_include_dirs(); print path[0]" OUTPUT_VARIABLE NUMPY_INCLUDEDIR OUTPUT_STRIP_TRAILING_WHITESPACE )
if (NOT NUMPY_INCLUDE)
    SET(NUMPY_INCLUDE ${NUMPY_INCLUDEDIR} CACHE PATH "Directory for the numpy headers. If it is not set automatically it is determined by python distutils." )
endif( NOT NUMPY_INCLUDE)
include_directories( ${NUMPY_INCLUDE} )
if( NOT EXISTS ${NUMPY_INCLUDE}/numpy/arrayobject.h )
    message( FATAL_ERROR "${NUMPY_INCLUDE}/numpy/arrayobject.h header is not found. If it is installed, than check the python flags and numpy header directories") 
endif( NOT EXISTS ${NUMPY_INCLUDE}/numpy/arrayobject.h )

# Include python and XMM header directories
include_directories(${PYTHON_INCLUDE_PATH})
add_definitions(-DUSE_PTHREAD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/libjson/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/models)

# Add target for python documentation from doxygen
set(DOC_DIR ../doc)
file(
    GLOB_RECURSE
    xmm_header_files
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.h
)
add_custom_command(OUTPUT xmm_doc.i DEPENDS doc ${xmm_header_files} COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/src/doxy2swig.py ${DOC_DIR}/xml/index.xml ${CMAKE_CURRENT_SOURCE_DIR}/xmm_doc.i)

set(CMAKE_SWIG_FLAGS "")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/numpy.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/xmm.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/xmm.i PROPERTIES SWIG_FLAGS "-DUSE_PTHREAD")
SET(SWIG_MODULE_xmm_EXTRA_DEPS 
    xmm_doc.i
)
# set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/xmm)
SWIG_ADD_MODULE(xmm python ${CMAKE_CURRENT_SOURCE_DIR}/src/xmm.i)
SWIG_LINK_LIBRARIES(xmm ${PYTHON_LIBRARIES} xmm pthread)

add_custom_command(TARGET _xmm POST_BUILD COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/src/move_files.sh)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/xmm)
