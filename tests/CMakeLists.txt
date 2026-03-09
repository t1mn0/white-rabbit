ADD_SUBDIRECTORY(queues/global)
ADD_SUBDIRECTORY(queues/local)
ADD_SUBDIRECTORY(coord)
# ADD_SUBDIRECTORY(...)

add_executable(rabbit_bench main.cpp)
target_link_libraries(rabbit_bench PRIVATE white_rabbit)

ENABLE_TESTING()
