
set(target logvis)


# External libraries

find_package(OpenGL REQUIRED)

message(STATUS "Tool ${target}")


# Includes

include_directories(
    ${OPENGL_INCLUDE_DIR}
)

include_directories(
    BEFORE
    ${CMAKE_SOURCE_DIR}/source/glbinding/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# Libraries

set(libs
    ${OPENGL_LIBRARIES}
    glbinding
)


# Compiler definitions

if (OPTION_BUILD_STATIC)
    add_definitions("-DGLBINDING_STATIC")
endif()


# Sources

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(api_includes
    ${include_path}/logvis_api.h
    ${include_path}/LogVis.h
)

set(sources
    ${source_path}/LogVis.cpp
    ${source_path}/RawFile.h
    ${source_path}/RawFile.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$|\\\\.inl$" 
    ${header_group} ${api_includes})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$" 
    ${source_group} ${sources})


# Build library

add_library(${target} ${api_includes} ${sources})

target_link_libraries(${target} ${libs} ${EXTRA_LIBS})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_FLAGS})

set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE              CXX
    FOLDER                      "${IDE_FOLDER}"
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    DEBUG_POSTFIX               "d${DEBUG_POSTFIX}"
    INCLUDE_PATH                ${include_path})


# Deployment


# Library

if(NOT OPTION_BUILD_STATIC)
    install(TARGETS ${target} COMPONENT runtime
        RUNTIME DESTINATION ${INSTALL_BIN}
        LIBRARY DESTINATION ${INSTALL_SHARED})
else()
    install(TARGETS ${target} COMPONENT dev
        ARCHIVE DESTINATION ${INSTALL_LIB})
endif()

# Header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE} COMPONENT dev)
