CMAKE_MINIMUM_REQUIRED(VERSION 3.30)

PROJECT(white_rabbit
  LANGUAGES CXX
)

# *---*---*---*---*---*---*

OPTION(WR_BUILD_TESTS "Build unit tests" ON)
OPTION(WR_BUILD_EXAMPLES "Build examples" ON)
OPTION(WR_BUILD_BENCHMARKS "Build benchmarks" ON)
OPTION(WR_ENABLE_ASAN "Enable Address Sanitizer" OFF)
OPTION(WR_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

# *---*---*---*---*---*---*

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# *---*---*---*---*---*---*

IF(MSVC)
  ADD_COMPILE_OPTIONS(/W4 /WX)
ELSE()
  ADD_COMPILE_OPTIONS(
    -Wall -Wextra -Wpedantic -Werror
  )
ENDIF()

IF(WR_ENABLE_ASAN AND WR_ENABLE_TSAN)
  MESSAGE(FATAL_ERROR "ASAN and TSAN are mutually exclusive")
ENDIF()

IF(WR_ENABLE_ASAN)
  MESSAGE(STATUS "Address Sanitizer Enabled")
  ADD_COMPILE_OPTIONS(-fsanitize=address)
  ADD_LINK_OPTIONS(-fsanitize=address)
ENDIF()

IF(WR_ENABLE_TSAN)
  MESSAGE(STATUS "Thread Sanitizer Enabled")
  ADD_COMPILE_OPTIONS(-fsanitize=thread)
  ADD_LINK_OPTIONS(-fsanitize=thread)
ENDIF()

# *---*---*---*---*---*---*

ADD_SUBDIRECTORY(third_party)

ADD_SUBDIRECTORY(src)

# *---*---*---*---*---*---*

IF(WR_BUILD_TESTS)
  enable_testing()
  ADD_SUBDIRECTORY(tests)
ENDIF()
