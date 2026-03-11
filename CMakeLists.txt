CMAKE_MINIMUM_REQUIRED(VERSION 3.30)

PROJECT(white_rabbit
  LANGUAGES CXX
)

# *---*---*---*---*---*---*

OPTION(WR_BUILD_TESTS "Build unit tests" OFF)
OPTION(WR_BUILD_EXAMPLES "Build examples" OFF)
OPTION(WR_BUILD_ANALYSIS "Build benchmarks" OFF)
OPTION(WR_ENABLE_TWIST "Build with Twist" OFF)
OPTION(WR_ENABLE_ASAN "Enable Address Sanitizer" OFF)
OPTION(WR_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

# *---*---*---*---*---*---*

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# *---*---*---*---*---*---*

ADD_SUBDIRECTORY(third_party)

# *---*---*---*---*---*---*

IF(MSVC)
  # Apply strict compiler warnings for MSVC
  # /W4: Set warning level 4
  # /WX: Treat warnings as errors
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
  MESSAGE(STATUS "[white-rabbit] : Address Sanitizer Enabled")
  ADD_COMPILE_OPTIONS(-fsanitize=address -g)
  ADD_LINK_OPTIONS(-fsanitize=address)
ENDIF()

IF(WR_ENABLE_TSAN)
  MESSAGE(STATUS "[white-rabbit] : Thread Sanitizer Enabled")
  ADD_COMPILE_OPTIONS(-fsanitize=thread -g)
  ADD_LINK_OPTIONS(-fsanitize=thread)
ENDIF()

# *---*---*---*---*---*---*

ADD_SUBDIRECTORY(src)

# *---*---*---*---*---*---*

IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  IF(WR_BUILD_TESTS OR WR_ENABLE_TWIST)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
  ENDIF()

  IF(WR_BUILD_ANALYSIS)
    ADD_SUBDIRECTORY(analysis)
  ENDIF()
ENDIF()
