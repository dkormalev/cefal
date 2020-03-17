include(CMakeFindDependencyMacro)

if(NOT TARGET cefal::cefal)
    include("${CMAKE_CURRENT_LIST_DIR}/cefal-targets.cmake")
endif()
