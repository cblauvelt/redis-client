include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# include source directories
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Make Unit Tests
set(HELPER_TEST "helper_test")
add_executable(${HELPER_TEST}
        "helper_functions_test.cpp"
)
target_include_directories(${HELPER_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${HELPER_TEST} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${HELPER_TEST} COMMAND $<TARGET_FILE:${HELPER_TEST}>)

set(REDIS_COMMAND_TEST "redis_command_test")
add_executable(${REDIS_COMMAND_TEST}
        "redis_command_test.cpp"
)
target_include_directories(${REDIS_COMMAND_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${REDIS_COMMAND_TEST} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${REDIS_COMMAND_TEST} COMMAND $<TARGET_FILE:${REDIS_COMMAND_TEST}>)

set(REDIS_VALUE_TEST "redis_value_test")
add_executable(${REDIS_VALUE_TEST}
        "redis_value_test.cpp"
)
target_include_directories(${REDIS_VALUE_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${REDIS_VALUE_TEST} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${REDIS_VALUE_TEST} COMMAND $<TARGET_FILE:${REDIS_VALUE_TEST}>)

set(REDIS_MESSAGE_TEST "redis_message_test")
add_executable(${REDIS_MESSAGE_TEST}
        "redis_message_test.cpp"
)
target_include_directories(${REDIS_MESSAGE_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${REDIS_MESSAGE_TEST} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${REDIS_MESSAGE_TEST} COMMAND $<TARGET_FILE:${REDIS_MESSAGE_TEST}>)

set(REDIS_REPLY_TEST "redis_reply_test")
add_executable(${REDIS_REPLY_TEST}
        "redis_reply_test.cpp"
)
target_include_directories(${REDIS_REPLY_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${REDIS_REPLY_TEST} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${REDIS_REPLY_TEST} COMMAND $<TARGET_FILE:${REDIS_REPLY_TEST}>)

if(${RUN_E2E_TESTS})
        set(REDIS_CLIENT_TEST "redis_client_test")
        add_executable(${REDIS_CLIENT_TEST}
                "redis_client_test.cpp"
        )
        target_include_directories(${REDIS_CLIENT_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(${REDIS_CLIENT_TEST} ${TARGET_NAME} ${CONAN_LIBS})
        add_test(NAME ${REDIS_CLIENT_TEST} COMMAND $<TARGET_FILE:${REDIS_CLIENT_TEST}>)

        set(REDIS_SUB_CONNECTION_TEST "redis_sub_connection_test")
        add_executable(${REDIS_SUB_CONNECTION_TEST}
                "redis_sub_connection_test.cpp"
        )
        target_include_directories(${REDIS_SUB_CONNECTION_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(${REDIS_SUB_CONNECTION_TEST} ${TARGET_NAME} ${CONAN_LIBS})
        add_test(NAME ${REDIS_SUB_CONNECTION_TEST} COMMAND $<TARGET_FILE:${REDIS_SUB_CONNECTION_TEST}>)

        set(REDIS_SUB_TEST "redis_sub_test")
        add_executable(${REDIS_SUB_TEST}
                "redis_sub_test.cpp"
        )
        target_include_directories(${REDIS_SUB_TEST} PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(${REDIS_SUB_TEST} ${TARGET_NAME} ${CONAN_LIBS})
        add_test(NAME ${REDIS_SUB_TEST} COMMAND $<TARGET_FILE:${REDIS_SUB_TEST}>)

endif(${RUN_E2E_TESTS})
