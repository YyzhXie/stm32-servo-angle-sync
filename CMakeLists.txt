cmake_minimum_required(VERSION 3.20)

project(stm32_servo_sync C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(SERVO_SYNC_BUILD_TESTS "Build host-side tests and simulation" ON)

add_library(servo_sync_core
    core/src/angle_frame.c
    core/src/angle_math.c
    core/src/angle_receiver.c
    core/src/moving_average.c
)

target_include_directories(servo_sync_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

target_compile_options(servo_sync_core
    PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
)

if(SERVO_SYNC_BUILD_TESTS)
    enable_testing()

    add_executable(test_core tests/test_core.c)
    target_link_libraries(test_core PRIVATE servo_sync_core)
    add_test(NAME core_unit_tests COMMAND test_core)

    add_executable(test_long_run tests/test_long_run.c)
    target_link_libraries(test_long_run PRIVATE servo_sync_core)
    add_test(NAME long_run_simulation COMMAND test_long_run)
endif()
