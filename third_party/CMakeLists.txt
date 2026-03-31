INCLUDE(FetchContent)

# *---*---*---*---*---*---*

FetchContent_Declare(
  ntrusive
    GIT_REPOSITORY https://github.com/niko256/ntrusive.git
    GIT_TAG master
)

# Fetch and make available
FetchContent_MakeAvailable(ntrusive)

IF(NOT TARGET third_party_ntrusive)
  add_library(third_party_ntrusive INTERFACE)
  target_include_directories(third_party_ntrusive INTERFACE
        ${ntrusive_SOURCE_DIR}/include
    )
ENDIF()


# *---*---*---*---*---*---*

IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  IF(WR_BUILD_TESTS OR WR_WITH_TWIST)
    # GTest
    FetchContent_Declare(
      googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
    )
    # ON: Sets the initial value to boolean True
    # CACHE: Stores the variable in CMakeCache.txt (persists across runs)
    # BOOL: Specifies the type of the variable
    # "...": A help string description
    # FORCE: Overwrites any existing value in the cache with the one specified here.
    SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  ENDIF()

  IF(WR_WITH_TWIST)
    MESSAGE(STATUS "[white-rabbit] : TWIST [ON] <:-:> [fault injection + simulation]")

    # Twist features
    SET(TWIST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    SET(TWIST_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    SET(TWIST_FAULTY ON CACHE BOOL "Enables Twist fault injection" FORCE)
    SET(TWIST_SIM ON CACHE BOOL "Enables Twist determ. simulation" FORCE)

    # Twist framework
    FetchContent_Declare(
      twist
        GIT_REPOSITORY https://gitlab.com/Lipovsky/twist.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(twist)
  ENDIF()
ENDIF()
