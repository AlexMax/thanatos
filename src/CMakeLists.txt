foreach(SUBDIR doom)
    add_subdirectory(${SUBDIR})
endforeach()

include_directories(${CMAKE_BINARY_DIR})

# Common source files used by absolutely everything:

set(COMMON_SOURCE_FILES
    i_main.cpp
    i_system.cpp  i_system.h
    m_argv.cpp    m_argv.h
    m_misc.cpp    m_misc.h)

# Dedicated server (chocolate-server):

set(DEDSERV_FILES
    d_dedicated.cpp
    d_mode.cpp        d_mode.h
    i_timer.cpp       i_timer.h
    net_common.cpp    net_common.h
    net_dedicated.cpp net_dedicated.h
    net_io.cpp        net_io.h
    net_packet.cpp    net_packet.h
    net_sdl.cpp       net_sdl.h
    net_query.cpp     net_query.h
    net_server.cpp    net_server.h
    net_structrw.cpp  net_structrw.h
    z_native.cpp      z_zone.h)

add_executable("${PROGRAM_PREFIX}server"
    ${COMMON_SOURCE_FILES} ${DEDSERV_FILES})
target_link_libraries("${PROGRAM_PREFIX}server"
    SDL2::SDL2 SDL2::net)

# Source files used by the game binaries (chocolate-doom, etc.)

set(GAME_SOURCE_FILES
    aes_prng.cpp      aes_prng.h
    array_view.cpp    array_view.h
    c_console.cpp     c_console.h
    c_font.cpp        c_font.h
    d_event.cpp       d_event.h
                    doomkeys.h
                    doomtype.h
    d_iwad.cpp        d_iwad.h
    d_loop.cpp        d_loop.h
    d_mode.cpp        d_mode.h
                    d_ticcmd.h
    deh_str.cpp       deh_str.h
    gusconf.cpp       gusconf.h
    i_cdmus.cpp       i_cdmus.h
    i_input.cpp       i_input.h
    i_joystick.cpp    i_joystick.h
                    i_swap.h
    i_sdlmusic.cpp
    i_sdlsound.cpp
    i_sound.cpp       i_sound.h
    i_timer.cpp       i_timer.h
    i_video.cpp       i_video.h
    i_videohr.cpp     i_videohr.h
    midifile.cpp      midifile.h
    mus2mid.cpp       mus2mid.h
    m_bbox.cpp        m_bbox.h
    m_cheat.cpp       m_cheat.h
    m_config.cpp      m_config.h
    m_controls.cpp    m_controls.h
    m_fixed.cpp       m_fixed.h
    net_client.cpp    net_client.h
    net_common.cpp    net_common.h
    net_dedicated.cpp net_dedicated.h
    net_defs.h
    net_io.cpp        net_io.h
    net_loop.cpp      net_loop.h
    net_packet.cpp    net_packet.h
    net_query.cpp     net_query.h
    net_sdl.cpp       net_sdl.h
    net_server.cpp    net_server.h
    net_structrw.cpp  net_structrw.h
    sha1.cpp          sha1.h
    memio.cpp         memio.h
    tables.cpp        tables.h
    v_diskicon.cpp    v_diskicon.h
    v_video.cpp       v_video.h
                    v_patch.h
    w_checksum.cpp    w_checksum.h
    w_main.cpp        w_main.h
    w_wad.cpp         w_wad.h
    w_file.cpp        w_file.h
    w_file_stdc.cpp
    w_file_posix.cpp
    w_file_win32.cpp
    w_merge.cpp       w_merge.h
    z_zone.cpp        z_zone.h)

set(DEHACKED_SOURCE_FILES
    deh_defs.h
    deh_io.cpp        deh_io.h
    deh_main.cpp      deh_main.h
    deh_mapping.cpp   deh_mapping.h
    deh_text.cpp)

# Some games support dehacked patches, some don't:

set(SOURCE_FILES ${COMMON_SOURCE_FILES} ${GAME_SOURCE_FILES})
set(SOURCE_FILES_WITH_DEH ${SOURCE_FILES} ${DEHACKED_SOURCE_FILES})

set(EXTRA_LIBS SDL2::SDL2 SDL2::mixer SDL2::net)
if(PNG_FOUND)
    list(APPEND EXTRA_LIBS png)
endif()
if(SAMPLERATE_FOUND)
    list(APPEND EXTRA_LIBS ${SAMPLERATE_LIBRARIES})
endif()

if(CMAKE_RC_COMPILER)
    add_executable("${PROGRAM_PREFIX}doom" ${SOURCE_FILES_WITH_DEH}
        resource.rc)
else()
    add_executable("${PROGRAM_PREFIX}doom" ${SOURCE_FILES_WITH_DEH})
endif()

target_link_libraries("${PROGRAM_PREFIX}doom" doom ${EXTRA_LIBS})

if(MSVC)
    # Suppress default manifest and use the one in the resource file instead.
    set_property(TARGET "${PROGRAM_PREFIX}doom" PROPERTY LINK_FLAGS "/MANIFEST:NO" APPEND)
endif()
