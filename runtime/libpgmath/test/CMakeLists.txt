#
# Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include(FindPythonInterp)
include(CheckTypeSize)
include(CheckLibraryExists)

if(NOT PYTHONINTERP_FOUND)
  libpgmath_warning_say("Could not find Python.")
  libpgmath_warning_say("The check-pgm target will not be available!")
  return()
endif()

check_library_exists(m sqrt "" LIBPGMATH_HAVE_LIBM)
set(LIBPGMATH_HAVE_LIBPGMATH_RUNTIME ${LIBPGMATH_RUNTIME_PATH})

macro(pythonize_bool var)
  if (${var})
    set(${var} True)
  else()
    set(${var} False)
  endif()
endmacro()

pythonize_bool(LIBPGMATH_HAVE_LIBM)
pythonize_bool(LIBPGMATH_HAVE_LIBPGMATH_RUNTIME)

set(LIBPGMATH_TEST_CFLAGS "" CACHE STRING
  "Extra compiler flags to send to the test compiler")

if(${LIBPGMATH_STANDALONE_BUILD})
  # Make sure we can use the console pool for recent cmake and ninja > 1.5
  if(CMAKE_VERSION VERSION_LESS 3.1.20141117)
    set(cmake_3_2_USES_TERMINAL)
  else()
    set(cmake_3_2_USES_TERMINAL USES_TERMINAL)
  endif()
  set(LIBPGMATH_TEST_COMPILER ${CMAKE_C_COMPILER} CACHE STRING
    "Compiler to use for testing Pgm library")
  set(LIBPGMATH_TEST_OPENMP_FLAG -fopenmp CACHE STRING
    "Compiler flag to use for testing Pgm library")
  find_program(LIBPGMATH_LLVM_LIT_EXECUTABLE
    NAMES llvm-lit lit.py lit
    PATHS ${LIBPGMATH_LLVM_TOOLS_DIR})
  if(NOT LIBPGMATH_LLVM_LIT_EXECUTABLE)
    libpgmath_say("Cannot find llvm-lit.")
    libpgmath_say("Please put llvm-lit in your PATH, set LIBPGMATH_LLVM_LIT_EXECUTABLE to its full path or point LIBPGMATH_LLVM_TOOLS_DIR to its directory")
    libpgmath_warning_say("The check-libpgmath target will not be available!")
    return()
  endif()
  # Set lit arguments
  # The -j 1 lets the actual tests run with the entire machine.
  # We have one test thread that spawns the tests serially.  This allows
  # Each test to use the entire machine.
  set(LIBPGMATH_LIT_ARGS_DEFAULT "-sv --show-unsupported --show-xfail")
  if(MSVC OR XCODE)
    set(LIBPGMATH_LIT_ARGS_DEFAULT "${LIBPGMATH_LIT_ARGS_DEFAULT} --no-progress-bar")
  endif()
  set(LIBPGMATH_LIT_ARGS "${LIBPGMATH_LIT_ARGS_DEFAULT}" CACHE STRING
    "Default options for lit")
  separate_arguments(LIBPGMATH_LIT_ARGS)
  add_custom_target(check-lib${LIBPGMATH_LIBRARY_NAME}
    COMMAND ${PYTHON_EXECUTABLE} ${LIBPGMATH_LLVM_LIT_EXECUTABLE} ${LIBPGMATH_LIT_ARGS} ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running libpgmath tests"
    ${cmake_3_2_USES_TERMINAL}
    DEPENDS ${LIBPGMATH_LIBRARY_NAME})
else()
  # LLVM source tree build, test just-built clang
  if(NOT MSVC)
    set(LIBPGMATH_TEST_COMPILER ${LLVM_RUNTIME_OUTPUT_INTDIR}/clang)
  else()
    set(LIBPGMATH_TEST_COMPILER ${LLVM_RUNTIME_OUTPUT_INTDIR}/clang.exe)
  endif()
 
  # Use add_lit_testsuite() from LLVM CMake.
  if(${LIBPGMATH_SYSTEM_PROCESSOR} MATCHES "x86_64")
    add_lit_testsuite(check-lib${LIBPGMATH_LIBRARY_NAME}
      "Running libpgmath tests"
      ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${LIBPGMATH_LIBRARY_NAME} pgarch)
  else()
    add_lit_testsuite(check-lib${LIBPGM_LIBRARY_NAME}
      "Running libpgm tests"
      ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${LIBPGM_LIBRARY_NAME})
  endif()
  
  add_lit_testsuite(check-lib${LIBPGMATH_LIBRARY_NAME}
    "Running libpgmath tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${LIBPGMATH_LIBRARY_NAME} pgarch)
endif()

if(${LIBPGMATH_SYSTEM_PROCESSOR} MATCHES "x86_64")
  set(LIBPGMATH_GETARCH_LIB pgarch)
  set(LIBPGMATH_GETARCH_LIB_FULL "${CMAKE_SHARED_LIBRARY_PREFIX}${LIBPGMATH_GETARCH_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(SRCS ${LIBPGMATH_TOOLS_DIR}/arch.c)
  add_library(${LIBPGMATH_GETARCH_LIB} SHARED ${SRCS})
  set_property(TARGET ${LIBPGMATH_GETARCH_LIB} PROPERTY COMPILE_DEFINITIONS TARGET_X8664 PG_PIC)
  # target_link_libraries(${LIBPGMATH_GETARCH_LIB} INTERFACE "-Wl,-e,lib_entry")
  target_include_directories(${LIBPGMATH_GETARCH_LIB} PUBLIC ${LIBPGMATH_SRC_DIR}/x86_64)
  add_dependencies(check-lib${LIBPGMATH_LIBRARY_NAME} ${LIBPGMATH_GETARCH_LIB})

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_property(TARGET ${LIBPGMATH_GETARCH_LIB} APPEND PROPERTY COMPILE_DEFINITIONS TARGET_LINUX_X8664)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_property(TARGET ${LIBPGMATH_GETARCH_LIB} APPEND PROPERTY COMPILE_DEFINITIONS TARGET_OSX_X8664)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_property(TARGET ${LIBPGMATH_GETARCH_LIB} APPEND PROPERTY COMPILE_DEFINITIONS TARGET_WIN_X8664)
    set_property(TARGET ${LIBPGMATH_GETARCH_LIB} PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  endif()
endif()

# Configure the lit.site.cfg.in file
set(AUTO_GEN_COMMENT "## Autogenerated by libpgmath configuration.\n# Do not edit!")
configure_file(lit.site.cfg.in lit.site.cfg @ONLY)
