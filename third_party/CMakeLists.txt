INCLUDE(FetchContent)

# *---*---*---*---*---*---*

# Horovod
FetchContent_Declare(
    vvv
    GIT_REPOSITORY https://gitlab.com/Lipovsky/vvv.git
    GIT_TAG master
)

FetchContent_MakeAvailable(
  vvv
)

IF (vvv_POPULATED)
    ADD_LIBRARY(third_party_vvv INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(third_party_vvv INTERFACE ${vvv_SOURCE_DIR}/source)
ENDIF()

# *---*---*---*---*---*---*

IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # GTest
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
  )

  SET(TWIST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  SET(TWIST_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

  # Twist framework
  FetchContent_Declare(
    twist
    GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
    GIT_TAG master
  )

  SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(
    googletest
    twist
  )
ENDIF()
