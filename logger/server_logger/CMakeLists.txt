cmake_minimum_required(VERSION 3.22)
project(mp_os_lggr_srvr_lggr)

include(FetchContent)

# Подключаем httplib (это просто заголовочный .h файл)
FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.12.1
)
FetchContent_MakeAvailable(httplib)

add_subdirectory(tests)

# Добавляем библиотеку
add_library(
        mp_os_lggr_srvr_lggr
        src/server_logger.cpp
        src/server_logger_builder.cpp
)

# Указываем include директории
target_include_directories(
        mp_os_lggr_srvr_lggr
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${httplib_SOURCE_DIR}
)

# Линкуем зависимости
target_link_libraries(
        mp_os_lggr_srvr_lggr
        PUBLIC
        mp_os_cmmn
        mp_os_lggr_lggr
        nlohmann_json::nlohmann_json
)
