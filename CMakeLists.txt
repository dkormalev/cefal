cmake_minimum_required(VERSION 3.13.0)

project(cefal VERSION 0.1)
set(CEFAL_BUILD_WITH_DUMMY 1)

add_library(cefal INTERFACE)
add_library(cefal::cefal ALIAS cefal)

target_include_directories(cefal INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(CEFAL_BUILD_WITH_DUMMY)
    message("-- cefal: Building with dummy unit")
    add_executable(cefal_dummy src/dummy.cpp)
    target_link_libraries(cefal_dummy PRIVATE cefal::cefal)
set_target_properties(cefal_dummy PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/cefal-config.cmake
    ${CMAKE_BINARY_DIR}/cefal-config.cmake
    INSTALL_DESTINATION lib/cmake/cefal
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/cefal
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)

install(FILES ${CMAKE_SOURCE_DIR}/include/cefal/cefal
    DESTINATION include/cefal
)

install(TARGETS cefal
   EXPORT cefal-targets
   RUNTIME DESTINATION lib
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)

install(EXPORT cefal-targets DESTINATION lib/cmake/cefal
   NAMESPACE cefal::
)

install(FILES ${CMAKE_BINARY_DIR}/cefal-config.cmake
   DESTINATION lib/cmake/cefal
)

export(EXPORT cefal-targets FILE ${CMAKE_BINARY_DIR}/cefal-targets.cmake NAMESPACE cefal::)

if(CEFAL_BUILD_TESTS)
    message("-- cefal: Building with tests")
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/tests/catch2)
    include(Catch)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
