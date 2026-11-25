#========================================================
# Project: HDR Ir-192 Brachytherapy Simulation
# Geant4 Version: 11.3.2
#========================================================

cmake_minimum_required(VERSION 3.16)

project(Ir192_Pb LANGUAGES CXX)

#========================================================
# C++ Standard
#========================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#========================================================
# Geant4 Configuration
#========================================================

find_package(Geant4 REQUIRED COMPONENTS ui_all vis_all)

include(${Geant4_USE_FILE})

#========================================================
# Include Directories
#========================================================

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

#========================================================
# Source and Header Files
#========================================================

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cc"
)

file(GLOB HEADERS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.hh"
)

#========================================================
# Executable
#========================================================

add_executable(Ir192_Pb
    "${PROJECT_SOURCE_DIR}/exampleB1.cc"
    ${SOURCES}
    ${HEADERS}
)

#========================================================
# Link Geant4 Libraries
#========================================================

target_link_libraries(Ir192_Pb
    PRIVATE
    ${Geant4_LIBRARIES}
)

#========================================================
# Copy Macro Files
#========================================================

if(EXISTS "${PROJECT_SOURCE_DIR}/macros")

    file(GLOB MACRO_FILES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/macros/*.mac"
    )

    foreach(MACRO_FILE ${MACRO_FILES})

        get_filename_component(
            MACRO_NAME
            "${MACRO_FILE}"
            NAME
        )

        configure_file(
            "${MACRO_FILE}"
            "${PROJECT_BINARY_DIR}/${MACRO_NAME}"
            COPYONLY
        )

    endforeach()

endif()

#========================================================
# Copy Data Files
#========================================================

if(EXISTS "${PROJECT_SOURCE_DIR}/data")

    file(COPY
        "${PROJECT_SOURCE_DIR}/data"
        DESTINATION "${PROJECT_BINARY_DIR}"
    )

endif()

#========================================================
# Installation
#========================================================

install(
    TARGETS Ir192_Pb
    RUNTIME DESTINATION bin
)

#========================================================
# Configuration Summary
#========================================================

message(STATUS "========================================")
message(STATUS "Project          : ${PROJECT_NAME}")
message(STATUS "Geant4 Version   : ${Geant4_VERSION}")
message(STATUS "C++ Standard     : ${CMAKE_CXX_STANDARD}")
message(STATUS "Source Directory : ${PROJECT_SOURCE_DIR}")
message(STATUS "Build Directory  : ${PROJECT_BINARY_DIR}")
message(STATUS "========================================")