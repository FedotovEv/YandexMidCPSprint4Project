
add_custom_target(unit_tests)

add_library(function
    function.cpp
)

add_library(file
    file.cpp
)

add_library(metric
    metric.cpp
    metric_impl/code_lines_count.cpp
    metric_impl/cyclomatic_complexity.cpp
    metric_impl/parameters_count.cpp
)

target_link_libraries(metric
    PUBLIC
        function
)

add_library(metric_accumulator
    metric_accumulator.cpp
    metric_accumulator_impl/average_accumulator.cpp
    metric_accumulator_impl/sum_average_accumulator.cpp
)

target_link_libraries(metric_accumulator
    PUBLIC
        function
        file
)

add_library(cmd_options
    cmd_options.cpp
)

target_link_libraries(cmd_options
    PUBLIC
        ${Boost_LIBRARIES}
)

add_subdirectory(metric_accumulator_impl)
add_subdirectory(metric_impl)