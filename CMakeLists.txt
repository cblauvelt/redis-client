cmake_minimum_required(VERSION 3.12)
project(redis-client)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
include(CheckCXXCompilerFlag)
include(conan.cmake)

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

# define project variables
set(TARGET_NAME ${PROJECT_NAME})

# include source directories
include_directories(
    "${CMAKE_SOURCE_DIR}/include"
)

if(${UNIX})
    message("Setting up for Unix Environment")

    check_cxx_compiler_flag(-std=c++20 HAVE_FLAG_STD_CXX20)
    if(HAVE_FLAG_STD_CXX20)

        set( THREAD_CXX_FLAGS "-pthread")

        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -std=c++20")

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

set(INCLUDE_FILES
    "include/redis_client.hpp"
    "include/redis_client_config.hpp"
    "include/redis_command.hpp"
    "include/redis_value.hpp"
    "include/redis_reply.hpp"
    "include/errors.hpp"
    "include/redis_error.hpp"
    "include/redis_message.hpp"
    "include/redis_subscriber.hpp"
    "include/redis_subscriber_connection.hpp"
    "include/redis_definitions.hpp"
)

set(SOURCE_FILES
    "src/commands.cpp"
    "src/errors.cpp"
    "src/helper_functions.cpp"
    "src/redis_client.cpp"
    "src/redis_command.cpp"
    "src/redis_value.cpp"
    "src/redis_reply.cpp"
    "src/redis_error.cpp"
    "src/redis_subscriber.cpp"
    "src/redis_subscriber_connection.cpp"
)

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
message("-- BuildTests is ${BuildTests}")
if(BuildTests)
	enable_testing()
    add_subdirectory(test)
endif(BuildTests)