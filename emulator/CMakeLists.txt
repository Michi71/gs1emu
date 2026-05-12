add_library(gs1emu STATIC
    src/delayline.cpp
    src/gs1emu.cpp
)

target_include_directories(gs1emu PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# C++ standard for Dexed compatibility
set_target_properties(gs1emu PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
