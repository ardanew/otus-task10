set(TARGET_NAME bulkserver_tests)
add_executable(${TARGET_NAME})

set(TESTS_SERVER_SOURCES ${SERVER_SOURCES})
list(TRANSFORM TESTS_SERVER_SOURCES PREPEND ../src/)
target_sources(${TARGET_NAME} PRIVATE
    test_bulk_server.cpp test_bulk_server.h
    ${TESTS_SERVER_SOURCES}
)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system locale)

target_compile_definitions(${TARGET_NAME} PRIVATE VIRTUAL_TESTS UNIT_TESTS)
if(MSVC)
    target_compile_definitions(${TARGET_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

target_link_libraries(${TARGET_NAME} 
    PRIVATE 
        GTest::gtest_main
        ${Boost_LIBRARIES}
)

if(MSVC) # for vs debugging
    set_property(TARGET ${TARGET_NAME} PROPERTY
        VS_DEBUGGER_WORKING_DIRECTORY $(OutDir)
    )
endif()

add_test(BulkServerTest "${CMAKE_INSTALL_PREFIX}/tests/bulk_server_tests")

include(GNUInstallDirs)
install(
    TARGETS ${TARGET_NAME} COMPONENT tests
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    RUNTIME DESTINATION tests
)
