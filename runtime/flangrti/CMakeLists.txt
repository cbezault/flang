
#
# Copyright (c) 2017-2019, NVIDIA CORPORATION.  All rights reserved.
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

enable_language(C ASM) # Enable assembly

set(ARCH_DEP_FILES
  ${TARGET_ARCHITECTURE}/flt_env.c
  ${TARGET_ARCHITECTURE}/dumpregs.c
)
include(CheckTypeSize)

set(CMAKE_EXTRA_INCLUDE_FILES "sys/ucontext.h")
CHECK_TYPE_SIZE("gregset_t" GREGSET_T LANGUAGE C)
set(CMAKE_EXTRA_INCLUDE_FILES )

configure_file(flangrti_config.h.in flangrti_config.h)

SET(ASM_OPTIONS "-DLINUX_ELF")
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

if( ${TARGET_ARCHITECTURE} STREQUAL "x86_64" )
  set(ARCH_DEP_FILES ${ARCH_DEP_FILES}
    x86_64/x86_daz.c
    x86_64/x86_flushz.c
  )
endif()

SET(PGC_SRC_FILES
  ${ARCH_DEP_FILES}
  bessel_tjn.c
  bessel_tyn.c
  dbessel_tjn.c
  dbessel_tyn.c
  f2cmain.c
  kidnnt.c
  ktrap.c
  mcopy1.c
  mcopy2.c
  mcopy4.c
  mcopy8.c
  mthi64.c
  mset1.c
  mset2.c
  mset4.c
  mset8.c
  mzero1.c
  mzero2.c
  mzero4.c
  mzero8.c
  ioargs.c
  fltmanip.c
  memalign.c
  iostdinit.c
  tempnam.c
  s_erf.c
  trace_lin.c
  trace.c
  idnint.c
  knint.c
  mpmalloc.c
  nint.c
  # src-mp files
  llcrit.c
  )

add_flang_library(flangrti_static
  ${PGC_SRC_FILES}
  ${SHARED_SOURCES}
  )
if (MSVC)
  set_property(TARGET flangrti_static PROPERTY OUTPUT_NAME libflangrti)
else()
  set_property(TARGET flangrti_static PROPERTY OUTPUT_NAME flangrti)
endif()


set(SHARED_LIBRARY TRUE)
add_flang_library(flangrti_shared
  ${PGC_SRC_FILES}
  ${SHARED_SOURCES}
  )

# Resolve symbols against libm
if (NOT MSVC)
  target_link_libraries(flangrti_shared PRIVATE m)
endif()


# Import OpenMP
if (NOT DEFINED LIBOMP_EXPORT_DIR)
  find_library( 
    FLANG_LIBOMP
    NAMES omp libomp
    HINTS ${CMAKE_BINARY_DIR}/lib)
  target_link_libraries(flangrti_shared PRIVATE ${FLANG_LIBOMP})
endif()

find_library( 
  LIBPGMATH
  NAMES pgmath libpgmath
  HINTS ${CMAKE_BINARY_DIR}/lib)
target_link_libraries(flangrti_shared PRIVATE ${LIBPGMATH})

if( ${TARGET_ARCHITECTURE} STREQUAL "aarch64" )
  target_compile_definitions(flangrti_static PRIVATE TARGET_LINUX_ARM)
  target_compile_definitions(flangrti_shared PRIVATE TARGET_LINUX_ARM)
elseif( ${TARGET_ARCHITECTURE} STREQUAL "ppc64le" )
  target_compile_definitions(flangrti_static PRIVATE TARGET_LINUX_POWER)
  target_compile_definitions(flangrti_shared PRIVATE TARGET_LINUX_POWER)
endif()

set_property(TARGET flangrti_shared PROPERTY OUTPUT_NAME flangrti)
set(SHARED_LIBRARY FALSE)

target_include_directories(flangrti_static
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

target_include_directories(flangrti_shared
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set_target_properties(flangrti_shared flangrti_static
                      PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${FLANG_RTE_LIB_DIR})

if (NOT MSVC)
  target_compile_options(flangrti_static PRIVATE -fPIC)
  target_compile_options(flangrti_shared PRIVATE -fPIC)
else()
  set_target_properties(flangrti_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

target_compile_options(flangrti_static PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-Mreentrant>)

target_compile_options(flangrti_shared PUBLIC $<$<COMPILE_LANGUAGE:Fortran>:-Mreentrant>)

