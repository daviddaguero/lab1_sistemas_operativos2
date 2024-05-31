# Request the minimum version of CMake, in case of lower version throws error.
# See: httos://cmake.org/cmake/help/latest/command/cmake_minimum_required.html
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

# Define your project:
# See: https://cmake.org/cmake/help/latest/command/project.html

project(
    "Lab1"
    VERSION 1.2.0
    DESCRIPTION "CMake C Project Template for Operating Systems II"
    LANGUAGES C
)

# Enable testing and the tests
include(CTest)

# Define the C standard, we are going to use std17
# See: https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_STANDARD.html
set(CMAKE_C_STANDARD 17)

# Include the 'include' directory, where the headers are
# See: https://cmake.org/cmake/help/latest/command/include_directories.html
include_directories(include)

# To automatic download external deps, we need to use 'FetchContent'
# See: https://cmake.org/cmake/help/latest/module/FetchContent.html
include(FetchContent) # Módulo de CMake que me permite descargar un repositorio

# In this example we are going to use Unity to download and to test our program
set(UNITY_GIT_URL "https://github.com/ThrowTheSwitch/Unity.git")

# Enable debug
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    Unity
    GIT_REPOSITORY ${UNITY_GIT_URL}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/Unity # Carpeta donde se hace el git clone
    UPDATE_COMMAND "" #Disable the update command
)

# Ensures the named dependencies have been populated, either by an earlier call or by populating them itself
# Detiene lo que viene configurando CMake hasta que los archivos esten descargados
FetchContent_MakeAvailable(Unity)

# Add the 'src' directory, where the source files are located
# See: https://cmake.org/cmake/help/latest/command/file.html#glob
 file(GLOB_RECURSE SOURCES "src/*.c")

# Define the source files
set(SERVER_SOURCE src/server.c src/cJSON.c)
set(TCP_CLIENT_SOURCE src/tcp_client.c src/cJSON.c)
set(UDP_CLIENT_SOURCE src/udp_client.c src/cJSON.c)

# Create the executable
# See: https://cmake.org/cmake/help/latest/command/add_executable.html
add_executable(server ${SERVER_SOURCE})
add_executable(tcp_client ${TCP_CLIENT_SOURCE})
add_executable(udp_client ${UDP_CLIENT_SOURCE})

# Compile the libraries
# See: https://cmake.org/cmake/help/latest/command/add_subdirectory.html
add_subdirectory(lib/libdyn)
add_subdirectory(lib/libsta)

# Include headers in lib
# See: https://cmake.org/cmake/help/latest/command/target_include_directories.html
target_include_directories(server PUBLIC lib/libdyn/include)
target_include_directories(server PUBLIC lib/libsta/include)

# Link the libraries
# See: https://cmake.org/cmake/help/latest/command/target_link_libraries.html
target_link_libraries(server possible_infection supplies_update DynamiCLib)

# Add subdirectory of tests
# RUN_TESTS y RUN_COVERAGE son variables que las vamos a estar pasando cuando ejecutemos el comando de CMake
if(RUN_TESTS EQUAL 1 OR RUN_COVERAGE EQUAL 1)
    # See: https://cmake.org/cmake/help/book/mastering-cmake/chapter/festing%20With%20CMake%20and%20CTest.html
    add_subdirectory(tests)
endif()

