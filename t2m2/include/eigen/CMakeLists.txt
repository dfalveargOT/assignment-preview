project(Eigen)

cmake_minimum_required(VERSION 2.6.2)


# guard against in-source builds

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. (you may need to remove CMakeCache.txt ")
endif()

#############################################################################
# retrieve version infomation                                               #
#############################################################################

# automatically parse the version number
file(READ "${CMAKE_SOURCE_DIR}/Eigen/src/Core/util/Macros.h" _eigen_version_header)
string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen_world_version_match "${_eigen_version_header}")
set(EIGEN_WORLD_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen_major_version_match "${_eigen_version_header}")
set(EIGEN_MAJOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen_minor_version_match "${_eigen_version_header}")
set(EIGEN_MINOR_VERSION "${CMAKE_MATCH_1}")
set(EIGEN_VERSION_NUMBER ${EIGEN_WORLD_VERSION}.${EIGEN_MAJOR_VERSION}.${EIGEN_MINOR_VERSION})

# if the mercurial program is absent, this will leave the EIGEN_HG_CHANGESET string empty,
# but won't stop CMake.
execute_process(COMMAND hg tip -R ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE EIGEN_HGTIP_OUTPUT)
execute_process(COMMAND hg branch -R ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE EIGEN_BRANCH_OUTPUT)

# if this is the default (aka development) branch, extract the mercurial changeset number from the hg tip output...
if(EIGEN_BRANCH_OUTPUT MATCHES "default")
string(REGEX MATCH "^changeset: *[0-9]*:([0-9;a-f]+).*" EIGEN_HG_CHANGESET_MATCH "${EIGEN_HGTIP_OUTPUT}")
set(EIGEN_HG_CHANGESET "${CMAKE_MATCH_1}")
endif(EIGEN_BRANCH_OUTPUT MATCHES "default")
#...and show it next to the version number
if(EIGEN_HG_CHANGESET)
  set(EIGEN_VERSION "${EIGEN_VERSION_NUMBER} (mercurial changeset ${EIGEN_HG_CHANGESET})")
else(EIGEN_HG_CHANGESET)
  set(EIGEN_VERSION "${EIGEN_VERSION_NUMBER}")
endif(EIGEN_HG_CHANGESET)


include(CheckCXXCompilerFlag)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#############################################################################
# find how to link to the standard libraries                                #
#############################################################################

find_package(StandardMathLibrary)

set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "")

if(NOT STANDARD_MATH_LIBRARY_FOUND)

  message(FATAL_ERROR
    "Can't link to the standard math library. Please report to the Eigen developers, telling them about your platform.")

else()

  if(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
    set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO} ${STANDARD_MATH_LIBRARY}")
  else()
    set(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO "${STANDARD_MATH_LIBRARY}")
  endif()

endif()

if(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
  message(STATUS "Standard libraries to link to explicitly: ${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO}")
else()
  message(STATUS "Standard libraries to link to explicitly: none")
endif()

option(EIGEN_BUILD_BTL "Build benchmark suite" OFF)
if(NOT WIN32)
  option(EIGEN_BUILD_PKGCONFIG "Build pkg-config .pc file for Eigen" ON)
endif(NOT WIN32)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
if(cmake_build_type_tolower STREQUAL "debug")
  set(CMAKE_BUILD_TYPE "Debug")
else()
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(EIGEN_SPLIT_LARGE_TESTS "Split large tests into smaller executables" ON)

option(EIGEN_DEFAULT_TO_ROW_MAJOR "Use row-major as default matrix storage order" OFF)
if(EIGEN_DEFAULT_TO_ROW_MAJOR)
  add_definitions("-DEIGEN_DEFAULT_TO_ROW_MAJOR")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fexceptions -fno-check-new -fno-common -fstrict-aliasing")
  set(CMAKE_CXX_FLAGS_DEBUG "-g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")

  check_cxx_compiler_flag("-Wno-variadic-macros" COMPILER_SUPPORT_WNOVARIADICMACRO)
  if(COMPILER_SUPPORT_WNOVARIADICMACRO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros")
  endif()

  check_cxx_compiler_flag("-Wextra" COMPILER_SUPPORT_WEXTRA)
  if(COMPILER_SUPPORT_WEXTRA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

  option(EIGEN_TEST_SSE2 "Enable/Disable SSE2 in tests/examples" OFF)
  if(EIGEN_TEST_SSE2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
    message("Enabling SSE2 in tests/examples")
  endif()

  option(EIGEN_TEST_SSE3 "Enable/Disable SSE3 in tests/examples" OFF)
  if(EIGEN_TEST_SSE3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
    message("Enabling SSE3 in tests/examples")
  endif()

  option(EIGEN_TEST_SSSE3 "Enable/Disable SSSE3 in tests/examples" OFF)
  if(EIGEN_TEST_SSSE3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
    message("Enabling SSSE3 in tests/examples")
  endif()

  option(EIGEN_TEST_SSE4_1 "Enable/Disable SSE4.1 in tests/examples" OFF)
  if(EIGEN_TEST_SSE4_1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    message("Enabling SSE4.1 in tests/examples")
  endif()

  option(EIGEN_TEST_SSE4_2 "Enable/Disable SSE4.2 in tests/examples" OFF)
  if(EIGEN_TEST_SSE4_2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
    message("Enabling SSE4.2 in tests/examples")
  endif()

  option(EIGEN_TEST_ALTIVEC "Enable/Disable AltiVec in tests/examples" OFF)
  if(EIGEN_TEST_ALTIVEC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maltivec -mabi=altivec")
    message("Enabling AltiVec in tests/examples")
  endif()

  option(EIGEN_TEST_NEON "Enable/Disable Neon in tests/examples" OFF)
  if(EIGEN_TEST_NEON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp -mfpu=neon -mcpu=cortex-a8")
    message("Enabling NEON in tests/examples")
  endif()

  check_cxx_compiler_flag("-fopenmp" COMPILER_SUPPORT_OPENMP)
  if(COMPILER_SUPPORT_OPENMP)
    option(EIGEN_TEST_OPENMP "Enable/Disable OpenMP in tests/examples" OFF)
    if(EIGEN_TEST_OPENMP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
      message("Enabling OpenMP in tests/examples")
    endif()
  endif()

endif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
  # C4127 - conditional expression is constant
  # C4714 - marked as __forceinline not inlined (I failed to deactivate it selectively)
  #         We can disable this warning in the unit tests since it is clear that it occurs
  #         because we are oftentimes returning objects that have a destructor or may
  #         throw exceptions - in particular in the unit tests we are throwing extra many
  #         exceptions to cover indexing errors.
  # C4505 - unreferenced local function has been removed (impossible to deactive selectively)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /wd4127 /wd4505 /wd4714")

  # replace all /Wx by /W4
  string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  check_cxx_compiler_flag("/openmp" COMPILER_SUPPORT_OPENMP)
  if(COMPILER_SUPPORT_OPENMP)
    option(EIGEN_TEST_OPENMP "Enable/Disable OpenMP in tests/examples" OFF)
    if(EIGEN_TEST_OPENMP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
      message("Enabling OpenMP in tests/examples")
    endif()
  endif()

  option(EIGEN_TEST_SSE2 "Enable/Disable SSE2 in tests/examples" OFF)
  if(EIGEN_TEST_SSE2)
    if(NOT CMAKE_CL_64)
      # arch is not supported on 64 bit systems, SSE is enabled automatically.
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
    endif(NOT CMAKE_CL_64)
    message("Enabling SSE2 in tests/examples")
  endif(EIGEN_TEST_SSE2)
endif(MSVC)

option(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION "Disable explicit vectorization in tests/examples" OFF)
if(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION)
  add_definitions(-DEIGEN_DONT_VECTORIZE=1)
  message("Disabling vectorization in tests/examples")
endif()

option(EIGEN_TEST_NO_EXPLICIT_ALIGNMENT "Disable explicit alignment (hence vectorization) in tests/examples" OFF)
if(EIGEN_TEST_NO_EXPLICIT_ALIGNMENT)
  add_definitions(-DEIGEN_DONT_ALIGN=1)
  message("Disabling alignment in tests/examples")
endif()

option(EIGEN_TEST_C++0x "Enables all C++0x features." OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# the user modifiable install path for header files
set(EIGEN_INCLUDE_INSTALL_DIR ${EIGEN_INCLUDE_INSTALL_DIR} CACHE PATH "The directory where we install the header files (optional)")

# set the internal install path for header files which depends on wether the user modifiable
# EIGEN_INCLUDE_INSTALL_DIR has been set by the user or not.
if(EIGEN_INCLUDE_INSTALL_DIR)
  set(INCLUDE_INSTALL_DIR
    ${EIGEN_INCLUDE_INSTALL_DIR}
    CACHE INTERNAL
    "The directory where we install the header files (internal)"
  )
else()
  set(INCLUDE_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/include/eigen3"
    CACHE INTERNAL
    "The directory where we install the header files (internal)"
  )
endif()

# similar to set_target_properties but append the property instead of overwriting it
macro(ei_add_target_property target prop value)

  get_target_property(previous ${target} ${prop})
  # if the property wasn't previously set, ${previous} is now "previous-NOTFOUND" which cmake allows catching with plain if()
  if(NOT previous)
    set(previous "")
  endif(NOT previous)
  set_target_properties(${target} PROPERTIES ${prop} "${previous} ${value}")
endmacro(ei_add_target_property)

install(FILES
  signature_of_eigen3_matrix_library
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
  )

if(EIGEN_BUILD_PKGCONFIG)
    configure_file(eigen3.pc.in eigen3.pc)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/eigen3.pc
        DESTINATION share/pkgconfig
        )
endif(EIGEN_BUILD_PKGCONFIG)

add_subdirectory(Eigen)

add_subdirectory(doc EXCLUDE_FROM_ALL)

include(CTest)
enable_testing() # must be called from the root CMakeLists, see man page
include(EigenTesting)
ei_init_testing()

if(EIGEN_LEAVE_TEST_IN_ALL_TARGET)
  add_subdirectory(test) # can't do EXCLUDE_FROM_ALL here, breaks CTest
else()
  add_subdirectory(test EXCLUDE_FROM_ALL)
endif()

add_subdirectory(unsupported)

add_subdirectory(demos EXCLUDE_FROM_ALL)

add_subdirectory(blas EXCLUDE_FROM_ALL)

# must be after test and unsupported, for configuring buildtests.in
add_subdirectory(scripts EXCLUDE_FROM_ALL)

# TODO: consider also replacing EIGEN_BUILD_BTL by a custom target "make btl"?
if(EIGEN_BUILD_BTL)
  add_subdirectory(bench/btl EXCLUDE_FROM_ALL)
endif(EIGEN_BUILD_BTL)

ei_testing_print_summary()

message("")
message("Configured Eigen ${EIGEN_VERSION_NUMBER}")
message("")

string(TOLOWER "${CMAKE_GENERATOR}" cmake_generator_tolower)
if(cmake_generator_tolower MATCHES "makefile")
  message("Some things you can do now:")
  message("--------------+----------------------------------------------------------------")
  message("Command       |   Description")
  message("--------------+----------------------------------------------------------------")
  message("make install  | Install to ${CMAKE_INSTALL_PREFIX}. To change that:")
  message("              |     cmake . -DCMAKE_INSTALL_PREFIX=yourpath")
  message("              |   Eigen headers will then be installed to:")
  message("              |     ${INCLUDE_INSTALL_DIR}")
  message("              |   To install Eigen headers to a separate location, do:")
  message("              |     cmake . -DEIGEN_INCLUDE_INSTALL_DIR=yourpath")
  message("make doc      | Generate the API documentation, requires Doxygen & LaTeX")
  message("make check    | Build and run the unit-tests. Read this page:")
  message("              |   http://eigen.tuxfamily.org/index.php?title=Tests")
  message("make blas     | Build BLAS library (not the same thing as Eigen)")
  message("--------------+----------------------------------------------------------------")
else()
  message("To build/run the unit tests, read this page:")
  message("  http://eigen.tuxfamily.org/index.php?title=Tests")
endif()

message("")
