file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HPP_FILES CONFIGURE_DEPENDS "*.hpp" "*.h")

add_library(white_rabbit ${CPP_FILES} ${HPP_FILES})

if(TARGET third_party_vvv)
    target_link_libraries(white_rabbit PRIVATE third_party_vvv)
endif()

target_include_directories(white_rabbit
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
