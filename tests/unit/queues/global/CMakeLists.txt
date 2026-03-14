ADD_EXECUTABLE(global_queue_tests
    unit.cc
)

TARGET_LINK_LIBRARIES(global_queue_tests
    PRIVATE
      white_rabbit
      GTest::gtest_main
)

ADD_TEST(NAME GlobalQUnitTests COMMAND global_queue_tests)
