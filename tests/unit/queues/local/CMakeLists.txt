SET(LOCAL_QUEUE_SOURCES
  wsq_unit.cc
  ringbuffer.cc
  # ${CMAKE_CURRENT_SOURCE_DIR}/sharedstate.cc
)

ADD_EXECUTABLE(local_queue_tests
  ${LOCAL_QUEUE_SOURCES}
)

TARGET_LINK_LIBRARIES(
  local_queue_tests
    PRIVATE
      white_rabbit
      GTest::gtest_main
)


TARGET_COMPILE_FEATURES(local_queue_tests
  PRIVATE
    cxx_std_20
)

INCLUDE(GoogleTest)
gtest_discover_tests(local_queue_tests)
