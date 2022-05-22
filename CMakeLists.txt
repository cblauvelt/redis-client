cmake_minimum_required(VERSION 3.12)
project(redis_client)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
include(CheckCXXCompilerFlag)
include(conan.cmake)

# define project variables
set(TARGET_NAME ${PROJECT_NAME})

# include source directories
include_directories(
    "${CMAKE_SOURCE_DIR}/"
)

if(${UNIX})
    message("-- Setting up for Unix Environment")

    check_cxx_compiler_flag(-std=c++20 HAVE_FLAG_STD_CXX20)
    if(HAVE_FLAG_STD_CXX20)

        set( THREAD_CXX_FLAGS "-pthread")
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
            message(FATAL_ERROR "Clang coroutines is not supported in asio. Please add support")
        endif()

        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${THREAD_CXX_FLAGS}" )

        if(NOT CMAKE_INSTALL_INCLUDEDIR)
            set( CMAKE_INSTALL_INCLUDEDIR "/usr/include" )
        endif()

        if(NOT CMAKE_INSTALL_LIBDIR)
            set( CMAKE_INSTALL_LIBDIR "/usr/lib" )
        endif()

    else()
        message(FATAL_ERROR "This library requires C++20")
    endif()

else(${UNIX})
    message( FATAL_ERROR "Non-unix build is not supported. Feel free to add one.")

endif(${UNIX})

# integrate conan build process
if(NOT EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_cmake_run(CONANFILE conanfile.py
                    BASIC_SETUP CMAKE_TARGETS
                    BUILD missing)

    conan_cmake_autodetect(settings)

endif()

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(FATAL_ERROR "The file conanbuildinfo.cmake could not be generated")
endif()

set(INCLUDE_FILES
    "redis/client_config.hpp"
    "redis/client.hpp"
    "redis/command.hpp"
    "redis/commands-json.hpp"
    "redis/commands.hpp"
    "redis/error.hpp"
    "redis/errors.hpp"
    "redis/helper_functions.hpp"
    "redis/message.hpp"
    "redis/reply.hpp"
    "redis/subscriber_connection.hpp"
    "redis/subscriber.hpp"
    "redis/types.hpp"
    "redis/value.hpp"
)

set(SOURCE_FILES
    "redis/client.cpp"
    "redis/command.cpp"
    "redis/commands.cpp"
    "redis/error.cpp"
    "redis/errors.cpp"
    "redis/helper_functions.cpp"
    "redis/reply.cpp"
    "redis/subscriber_connection.cpp"
    "redis/subscriber.cpp"
    "redis/value.cpp"
)

# Extra debugging
option(CPOOL_TRACE_LOGGING "Extended cpool debug prints" OFF)
message("-- CPOOL_TRACE_LOGGING is ${CPOOL_TRACE_LOGGING}")
if(CPOOL_TRACE_LOGGING)
    add_compile_definitions(CPOOL_TRACE_LOGGING)
endif(CPOOL_TRACE_LOGGING)

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${TARGET_NAME} ${CONAN_LIBS})

# install headers
set(include_install_dir "${CMAKE_INSTALL_INCLUDEDIR}/redis")
set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
install(
    DIRECTORY "${HEADER_BASE}"
    DESTINATION "${include_install_dir}"
)
# install libs
set(lib_install_dir "${CMAKE_INSTALL_LIBDIR}")
set(LIB_BASE "${CMAKE_BINARY_DIR}/lib/")
install(
    DIRECTORY "${LIB_BASE}"
    DESTINATION "${lib_install_dir}"
)

# create and configure the unit test target
option(BuildTests "Build the unit tests" ON)
option(RUN_E2E_TESTS "Run e2e tests" OFF)
message("-- BuildTests is ${BuildTests}")
message("-- RUN_E2E_TESTS is ${RUN_E2E_TESTS}")
if(BuildTests)
	enable_testing()
    add_subdirectory(test)
endif(BuildTests)