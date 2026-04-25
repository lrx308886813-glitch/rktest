cmake_minimum_required(VERSION 3.16)
project(elder-care-terminal VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ECT_BUILD_TESTS "Build tests" ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ECT_SOURCES
    src/core/App.cpp
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/StateMachine.cpp
    src/core/Storage.cpp
    src/services/ReminderService.cpp
    src/services/MessageService.cpp
    src/services/ContactService.cpp
    src/services/AudioService.cpp
    src/services/CameraService.cpp
    src/services/CallService.cpp
    src/services/DeviceService.cpp
    src/hardware/MockAudio.cpp
    src/hardware/MockCamera.cpp
    src/hardware/MockGpio.cpp
    src/hardware/Rk3566HardwarePlaceholder.cpp
    src/web/HttpServer.cpp
)

add_library(ect_core STATIC ${ECT_SOURCES})
target_include_directories(ect_core PUBLIC include)
target_compile_options(ect_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
if (WIN32)
    target_link_libraries(ect_core PUBLIC ws2_32)
else()
    target_link_libraries(ect_core PUBLIC pthread)
endif()

add_executable(elder-care-terminal src/main.cpp)
target_link_libraries(elder-care-terminal PRIVATE ect_core)

add_executable(factory_test tools/factory_test.cpp)
target_link_libraries(factory_test PRIVATE ect_core)

if (ECT_BUILD_TESTS)
    enable_testing()
    add_executable(test_state_machine tests/test_state_machine.cpp)
    target_link_libraries(test_state_machine PRIVATE ect_core)
    add_test(NAME state_machine COMMAND test_state_machine)

    add_executable(test_reminder_service tests/test_reminder_service.cpp)
    target_link_libraries(test_reminder_service PRIVATE ect_core)
    add_test(NAME reminder_service COMMAND test_reminder_service)

    add_executable(test_config tests/test_config.cpp)
    target_link_libraries(test_config PRIVATE ect_core)
    add_test(NAME config COMMAND test_config)
endif()

install(TARGETS elder-care-terminal RUNTIME DESTINATION bin)
install(DIRECTORY config web deploy docs DESTINATION .)
