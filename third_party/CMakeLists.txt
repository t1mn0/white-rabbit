INCLUDE(FetchContent)

# GTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)

# Horovod
FetchContent_Declare(
    vvv
    GIT_REPOSITORY https://gitlab.com/Lipovsky/vvv.git
    GIT_TAG master
)

FetchContent_MakeAvailable(
  googletest
  vvv
)

IF (vvv_POPULATED)
    ADD_LIBRARY(third_party_vvv INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(third_party_vvv INTERFACE ${vvv_SOURCE_DIR}/source)
ENDIF()
