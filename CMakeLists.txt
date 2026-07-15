cmake_minimum_required(VERSION 3.16)

project(Ir192_Pb)

#========================================================
# Geant4
#========================================================
find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

#========================================================
# Headers & Sources
#========================================================
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

file(GLOB headers
    ${PROJECT_SOURCE_DIR}/include/*.hh
)

#========================================================
# Executable
#========================================================
add_executable(Ir192_Pb
    exampleB1.cc
    ${sources}
)

target_link_libraries(Ir192_Pb
    ${Geant4_LIBRARIES}
)

#========================================================
# Copy macro files
#========================================================
set(MACRO_FILES
    run.mac
)

foreach(file ${MACRO_FILES})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${file}
        ${PROJECT_BINARY_DIR}/${file}
        COPYONLY
    )
endforeach()

#========================================================
# Copy data folder
#========================================================
file(COPY
    ${PROJECT_SOURCE_DIR}/data
    DESTINATION ${PROJECT_BINARY_DIR}
)

#========================================================
# Install
#========================================================
install(TARGETS Ir192_Pb DESTINATION bin)