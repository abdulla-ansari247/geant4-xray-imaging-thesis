cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(XraySimulation)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(sim ${sources} ${PROJECT_SOURCE_DIR}/sim.cc)

target_link_libraries(sim ${Geant4_LIBRARIES})
target_include_directories(sim PRIVATE ${PROJECT_SOURCE_DIR}/include)

# ------------------------------------------------------------
# Copy macros into the build directory so /control/execute works
# ------------------------------------------------------------
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/macro/*.mac")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/macro")

foreach(_mac ${MACRO_FILES})
  configure_file(${_mac} "${PROJECT_BINARY_DIR}/macro/" COPYONLY)
endforeach()

add_custom_target(XraySimulation DEPENDS sim)
