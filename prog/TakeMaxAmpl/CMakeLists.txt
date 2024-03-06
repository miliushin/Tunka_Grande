#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 2.4 FATAL_ERROR)
project(calib)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net)
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
# Setup include directory for this project
include(${ROOT_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hpp)
#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(TakeMaxAmpl main.cpp ${sources} ${headers})
target_link_libraries(TakeMaxAmpl   ${ROOT_LIBRARIES})
#----------------------------------------------------------------------------
add_custom_target(calib DEPENDS TakeMaxAmpl)
#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS TakeMaxAmpl DESTINATION bin)
