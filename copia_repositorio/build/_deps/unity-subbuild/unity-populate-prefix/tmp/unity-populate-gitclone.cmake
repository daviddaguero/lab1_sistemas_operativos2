# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if(EXISTS "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitclone-lastrun.txt" AND EXISTS "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitinfo.txt" AND
  "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitclone-lastrun.txt" IS_NEWER_THAN "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitinfo.txt")
  message(STATUS
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitclone-lastrun.txt'"
  )
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external/Unity"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external/Unity'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/bin/git"
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/ThrowTheSwitch/Unity.git" "Unity"
    WORKING_DIRECTORY "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external"
    RESULT_VARIABLE error_code
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/ThrowTheSwitch/Unity.git'")
endif()

execute_process(
  COMMAND "/bin/git"
          checkout "master" --
  WORKING_DIRECTORY "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external/Unity"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'master'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external/Unity"
    RESULT_VARIABLE error_code
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/external/Unity'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitinfo.txt" "/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/_deps/unity-subbuild/unity-populate-prefix/src/unity-populate-stamp/unity-populate-gitclone-lastrun.txt'")
endif()