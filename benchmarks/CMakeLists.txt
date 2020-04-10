cmake_minimum_required(VERSION 3.13.0)

macro(cefal_benchmark dir target)
    set(target_name benchmark_${dir}_${target})
    add_executable(${target_name} ${dir}/benchmark_${target}.cpp helpers/benchmark_main.cpp)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
    target_compile_options(${target_name} PRIVATE -Wno-unused-variable)
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty ${CMAKE_SOURCE_DIR}/benchmarks/helpers)
    target_link_libraries(${target_name} cefal::cefal)

    add_custom_target(run-${target_name}
        COMMAND ${CMAKE_BINARY_DIR}/benchmarks/${target_name}
        USES_TERMINAL)
    add_dependencies(run-benchmarks run-${target_name})
endmacro()

add_custom_target(run-benchmarks)

cefal_benchmark(functor map_std_containers)
cefal_benchmark(functor map_std_ranges)

cefal_benchmark(filterable filter_std_containers)
cefal_benchmark(filterable filter_std_ranges)
