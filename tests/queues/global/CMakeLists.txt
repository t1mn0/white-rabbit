ADD_EXECUTABLE(unit_tests
    unit.cpp
)

TARGET_LINK_LIBRARIES(unit_tests
    PRIVATE
    white_rabbit
    GTest::gtest_main
)

ADD_TEST(NAME UnitTests COMMAND unit_tests)
