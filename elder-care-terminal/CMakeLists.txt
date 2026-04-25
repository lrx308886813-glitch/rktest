cmake_minimum_required(VERSION 3.16)
project(elder_care_terminal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(elder_care_core
    src/core/logger.cpp
    src/core/state_machine.cpp
    src/core/reminder_engine.cpp
    src/services/audio_service.cpp
    src/services/data_store.cpp
    src/hardware/mock_hardware.cpp
    src/hardware/rk3566_placeholder.cpp
    src/web/http_server.cpp
)
target_include_directories(elder_care_core PUBLIC include)

add_executable(elder_care_terminal src/main.cpp)
target_link_libraries(elder_care_terminal PRIVATE elder_care_core pthread)

add_executable(factory_test tools/factory_test.cpp)
target_link_libraries(factory_test PRIVATE elder_care_core pthread)
