ADD_EXECUTABLE(globalq_unit_tests
    unit.cpp
)

TARGET_LINK_LIBRARIES(globalq_unit_tests
    PRIVATE
    white_rabbit
    GTest::gtest_main
)

ADD_TEST(NAME GlobalQUnitTests COMMAND globalq_unit_tests)
