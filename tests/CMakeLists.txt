ADD_SUBDIRECTORY(queues/global)
ADD_SUBDIRECTORY(queues/local)
ADD_SUBDIRECTORY(coord)
# ADD_SUBDIRECTORY(...)

ADD_EXECUTABLE(indexing_fix indexing_fix.cpp)

add_executable(rabbit_bench main.cpp)
target_link_libraries(rabbit_bench PRIVATE white_rabbit)

TARGET_LINK_LIBRARIES(indexing_fix
    PRIVATE
    white_rabbit
    GTest::gtest
)

ENABLE_TESTING()
