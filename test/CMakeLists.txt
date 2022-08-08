include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# include source directories
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Make Unit Tests
set(UNIT_TESTS "unit_tests")
add_executable(${UNIT_TESTS}
        "helper_functions_test.cpp"
        "redis_command_test.cpp"
        "redis_value_test.cpp"
        "redis_message_test.cpp"
        "redis_reply_test.cpp"
)
target_include_directories(${UNIT_TESTS} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${UNIT_TESTS} ${TARGET_NAME} ${CONAN_LIBS})
add_test(NAME ${UNIT_TESTS} COMMAND $<TARGET_FILE:${UNIT_TESTS}>)

if(${RUN_E2E_TESTS})
        set(E2E_TESTS "e2e_tests")
        add_executable(${E2E_TESTS}
                "redis_sub_connection_test.cpp"
                "redis_client_test.cpp"
                "redis_sub_test.cpp"
        )
        target_include_directories(${E2E_TESTS} PRIVATE ${CMAKE_SOURCE_DIR})
        target_link_libraries(${E2E_TESTS} ${TARGET_NAME} ${CONAN_LIBS})
        add_test(NAME ${E2E_TESTS} COMMAND $<TARGET_FILE:${E2E_TESTS}>)

endif(${RUN_E2E_TESTS})
