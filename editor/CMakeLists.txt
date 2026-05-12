add_executable(gs1_editor
    src/main.cpp
    src/tui.cpp
    src/curve_generator.cpp
    src/sysex.cpp
    src/audio_engine.cpp
)

target_include_directories(gs1_editor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(gs1_editor
    gs1emu
    ${SDL2_LIBRARIES}
    ${PORTMIDI_LIBRARY}
    ncurses
)

set_target_properties(gs1_editor PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
