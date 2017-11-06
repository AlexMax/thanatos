set(RESOURCE_FILES)

# Graphics
foreach(index RANGE 032 126)
    if(${index} LESS 100])
        list(APPEND RESOURCE_FILES "graphic/CONFN0${index}.lmp")
    else()
        list(APPEND RESOURCE_FILES "graphic/CONFN${index}.lmp")
    endif()
endforeach()

# Turn resources into headers
set(RESOURCE_HEADERS)
foreach(resource ${RESOURCE_FILES})
    get_filename_component(resource_dir "${resource}" DIRECTORY)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${resource_dir}")
    set(resource_out "${CMAKE_CURRENT_BINARY_DIR}/${resource}.h")
    add_custom_command(
        OUTPUT ${resource_out}
        COMMAND $<TARGET_FILE:file2cpp> ${resource_out} "${CMAKE_CURRENT_SOURCE_DIR}/${resource}"
        DEPENDS file2cpp "${resource}" VERBATIM)
    list(APPEND RESOURCE_HEADERS ${resource_out})
endforeach()

add_custom_target(resource
    DEPENDS ${RESOURCE_HEADERS}
    SOURCES ${RESOURCE_FILES})
# target_include_directories(resource INTERFACE ${CMAKE_CURRENT_BINARY_DIR})