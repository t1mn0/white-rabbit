ADD_EXECUTABLE(coord_tests
    unit.cc
)

TARGET_LINK_LIBRARIES(coord_tests
    PRIVATE
    white_rabbit
    GTest::gtest_main
)

ADD_TEST(NAME CoordUnitTests COMMAND coord_tests)
