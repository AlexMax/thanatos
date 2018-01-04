include_directories(${CMAKE_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_executable(${PROGRAM_PREFIX}midiproc
    buffer.c buffer.h
    main.c
             proto.h)
target_link_libraries(${PROGRAM_PREFIX}midiproc SDL2::SDL2 SDL2::mixer)
