set(BUILD_TARGET "msaa")

configure_file(${CMAKE_CURRENT_LIST_DIR}/directories.h.in
               ${CMAKE_CURRENT_LIST_DIR}/directories.h @ONLY)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SOURCE_FILES "*.cpp" "*.h")
add_executable(${BUILD_TARGET} ${SOURCE_FILES})
qt5_use_modules(${BUILD_TARGET} Widgets OpenGL Xml)

target_link_libraries(${BUILD_TARGET} ${OPENGL_LIBRARIES} ${QT_LIBRARIES})

source_group("Source Files" FILES ${SOURCE_FILES})

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")
endif()
