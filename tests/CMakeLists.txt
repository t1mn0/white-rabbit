ADD_SUBDIRECTORY(queues/global)
ADD_SUBDIRECTORY(queues/local)
# ADD_SUBDIRECTORY(...)

ADD_EXECUTABLE(indexing_fix indexing_fix.cpp)

TARGET_LINK_LIBRARIES(indexing_fix
    PRIVATE
    white_rabbit
    GTest::gtest
)

ENABLE_TESTING()
