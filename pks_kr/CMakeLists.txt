cmake_minimum_required(VERSION 3.16)

project(pks_kr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)

add_executable(pks_kr
        src/game/main.cpp
        src/weapons/revolver.cpp
        src/contestants/contestant.cpp
        src/contestants/nn.cpp
        src/contestants/player.cpp
        src/API/telegram.cpp
        src/game/game.cpp
        src/secret/dotenv.cpp
)

target_include_directories(pks_kr PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIR}
)

if (CURL_FOUND)
    target_include_directories(pks_kr PRIVATE ${CURL_INCLUDE_DIRS})
    target_compile_definitions(pks_kr PRIVATE HAVE_CURL)
endif()

target_link_libraries(pks_kr PRIVATE
        Threads::Threads
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
)

if (WIN32)
    target_link_libraries(pks_kr PRIVATE ws2_32)
endif()

message(STATUS "OpenSSL: ${OPENSSL_INCLUDE_DIR}, ${OPENSSL_LIBRARIES}")
message(STATUS "Boost: ${Boost_INCLUDE_DIR}, ${Boost_LIBRARIES}")
message(STATUS "CURL: ${CURL_INCLUDE_DIRS}, ${CURL_LIBRARIES}")

option(PKS_KR_BUILD_TESTS "Build unit tests" ON)

if (PKS_KR_BUILD_TESTS)
    enable_testing()

    include(FetchContent)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.4
    )

    FetchContent_MakeAvailable(Catch2)

    add_subdirectory(tests)
endif()