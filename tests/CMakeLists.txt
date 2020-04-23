cmake_minimum_required(VERSION 3.13.0)

macro(cefal_test dir target)
    set(target_name test_${dir}_${target})
    add_executable(${target_name} ${dir}/test_${target}.cpp helpers/test_main.cpp)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
    target_compile_options(${target_name} PRIVATE -Wno-unused-variable)
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty ${CMAKE_SOURCE_DIR}/tests/helpers)
    target_link_libraries(${target_name} cefal::cefal)
    catch_discover_tests(${target_name} TEST_PREFIX "${dir}/${target}: .")
endmacro()

cefal_test(monoid basic_types)
cefal_test(monoid std_optional)
cefal_test(monoid with_functions)
cefal_test(monoid std_containers)

cefal_test(foldable with_functions)
cefal_test(foldable std_ranges)
cefal_test(foldable std_containers)

cefal_test(functor from_foldable)
cefal_test(functor std_optional)
cefal_test(functor std_ranges)
cefal_test(functor with_functions)

cefal_test(monad from_foldable)
cefal_test(monad std_optional)
cefal_test(monad with_functions)

cefal_test(filterable from_foldable)
cefal_test(filterable std_optional)
cefal_test(filterable std_ranges)
cefal_test(filterable with_functions)

cefal_test(converter from_self)
cefal_test(converter from_std_containers)
cefal_test(converter from_std_optional)

