cmake_minimum_required(VERSION 3.22)

include(FetchContent)

# Подключаем Crow через FetchContent
FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# Подключаем GoogleTest (если еще не подключен где-то выше)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Первый тест — mp_os_lggr_srvr_lggr_tests
add_executable(
        mp_os_lggr_srvr_lggr_tests
        server_logger_tests.cpp
)

target_link_libraries(
        mp_os_lggr_srvr_lggr_tests
        PRIVATE
        mp_os_lggr_srvr_lggr
        gtest_main
        Crow::Crow
)

add_test(NAME mp_os_lggr_srvr_lggr_tests COMMAND mp_os_lggr_srvr_lggr_tests)

# Второй тест — serv_test
add_executable(
        serv_test
        server.cpp
        server.h
        server_test.cpp
)

target_link_libraries(
        serv_test
        PRIVATE
        mp_os_lggr_srvr_lggr
        gtest_main
        Crow::Crow
)

add_test(NAME serv_test COMMAND serv_test)
