cmake_minimum_required(3.10)

project(Montecarlo Quantumdots)

add_subdirectory(code/utility)
add_subdirectory(code/ui_layer)
add_subdirectory(code/experiment_layer)
add_subdirectory(code/experiment_layer/quantumdot)
add_subdirectory(code/experiment_management_layer)

add_executable(/bootstrap_code/SGQD_test.cpp)

target_link_libraries(Montecarlo Quantumdots PUBLIC code/utility)
target_link_libraries(Montecarlo Quantumdots PUBLIC code/utility)
target_link_libraries(Montecarlo Quantumdots PUBLIC code/ui_layer)
target_link_libraries(Montecarlo Quantumdots PUBLIC code/experiment_layer)
target_link_libraries(Montecarlo Quantumdots PUBLIC code/experiment_layer/quantumdot)
target_link_libraries(Montecarlo Quantumdots PUBLIC code/experiment_management_layer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

