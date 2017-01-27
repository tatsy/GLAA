cmake_minimum_required(VERSION 3.0.0)
project(MSAA)

# -----------------------------------------------------------------------------
# Library prefix and suffix
# -----------------------------------------------------------------------------
if (WIN32)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".lib")
elseif(OSX)
  set(LIB_PREFIX "")
  set(LIB_SUFFIX ".dylib")
else()
  set(LIB_PREFIX "lib")
  set(LIB_SUFFIX ".so")
endif()

# -----------------------------------------------------------------------------
# Required packages
# -----------------------------------------------------------------------------
if (WIN32)
  set(QT5_ROOT "QT5-NOT_FOUND" CACHE PATH "")
  if (NOT EXISTS "${QT5_ROOT}/lib/cmake")
    message(FATAL_ERROR "[ERROR] Qt5 not found!!")
  endif()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT5_ROOT})
endif()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(OpenGL REQUIRED)

if (MSVC)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

# -----------------------------------------------------------------------------
# Set output paths
# -----------------------------------------------------------------------------
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# -----------------------------------------------------------------------------
# Create folder for outputs
# -----------------------------------------------------------------------------
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/output)

# -----------------------------------------------------------------------------
# Add standard definitions
# -----------------------------------------------------------------------------
if (MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  add_definitions("/D_CRT_SECURE_NO_WARNINGS /W3 /wd4251")
endif()

# -----------------------------------------------------------------------------
# Process subdirectories
# -----------------------------------------------------------------------------
add_subdirectory(sources)
