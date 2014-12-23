#==================================================================================================#
#                                                                                                  #
#  Copyright 2013 MaidSafe.net limited                                                             #
#                                                                                                  #
#  This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,        #
#  version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which    #
#  licence you accepted on initial access to the Software (the "Licences").                        #
#                                                                                                  #
#  By contributing code to the MaidSafe Software, or to this project generally, you agree to be    #
#  bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root        #
#  directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available   #
#  at: http://www.maidsafe.net/licenses                                                            #
#                                                                                                  #
#  Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed    #
#  under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF   #
#  ANY KIND, either express or implied.                                                            #
#                                                                                                  #
#  See the Licences for the specific language governing permissions and limitations relating to    #
#  use of the MaidSafe Software.                                                                   #
#                                                                                                  #
#==================================================================================================#
#                                                                                                  #
#  Module used to set Common compiler and linker flags.                                            #
#                                                                                                  #
#==================================================================================================#


target_compile_definitions(maidsafe_common
  PUBLIC
    APPLICATION_VERSION_MAJOR=${ApplicationVersionMajor}
    APPLICATION_VERSION_MINOR=${ApplicationVersionMinor}
    APPLICATION_VERSION_PATCH=${ApplicationVersionPatch}
    TARGET_PLATFORM=${TargetPlatform}
    TARGET_ARCHITECTURE=${TargetArchitecture}
    $<$<BOOL:${INCLUDE_TESTS}>:TESTING>
    $<$<BOOL:${PROFILING}>:USE_PROFILING>
    $<$<BOOL:${USE_LOGGING}>:USE_LOGGING=1>
    $<$<BOOL:${DONT_USE_LOGGING}>:USE_LOGGING=0>
    $<$<BOOL:${VLOGGING}>:USE_VLOGGING>
    ASIO_HAS_MOVE
    ASIO_HAS_STD_ARRAY
    ASIO_HAS_STD_ATOMIC
    ASIO_HAS_STD_SHARED_PTR
    ASIO_HAS_STD_CHRONO
    ASIO_HAS_STD_SYSTEM_ERROR
    ASIO_HAS_VARIADIC_TEMPLATES
    BOOST_ASIO_HAS_MOVE
    BOOST_ASIO_HAS_STD_ARRAY
    BOOST_ASIO_HAS_STD_ATOMIC
    BOOST_ASIO_HAS_STD_SHARED_PTR
    BOOST_ASIO_HAS_STD_CHRONO
    BOOST_ASIO_HAS_STD_SYSTEM_ERROR
    BOOST_ASIO_HAS_VARIADIC_TEMPLATES
    BOOST_FILESYSTEM_NO_DEPRECATED
    BOOST_FILESYSTEM_VERSION=3
    BOOST_PYTHON_STATIC_LIB
    BOOST_THREAD_VERSION=4
    BOOST_RESULT_OF_USE_DECLTYPE
    $<$<CXX_COMPILER_ID:Clang>:ASIO_HAS_CLANG_CXX BOOST_ASIO_HAS_CLANG_CXX>
    $<$<BOOL:${WIN32}>:MAIDSAFE_WIN32>
    $<$<BOOL:${CMAKE_CL_64}>:MAIDSAFE_WIN64>
    $<$<BOOL:${MSVC}>:
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0600
        _UNICODE
        UNICODE
        _BIND_TO_CURRENT_VCLIBS_VERSION=1
        _VARIADIC_MAX=10  # VC11 contains std::tuple with variadic templates emulation macro.  _VARIADIC_MAX defaulted to 5 but gtest requires 10.
        NOMINMAX          # Prevents std::min() and std::max() from being overwritten.
        BOOST_ALL_NO_LIB  # Prevents automatic linking of boost libraries.
    >
    $<$<BOOL:${UNIX}>:
        _FILE_OFFSET_BITS=64
        HAVE_PTHREAD
        $<$<BOOL:${APPLE}>:MAIDSAFE_APPLE>
        $<$<BOOL:${BSD}>:MAIDSAFE_BSD>
        $<$<NOT:$<BOOL:${APPLE}>>:MAIDSAFE_LINUX>  # Also defined on BSD (ugly, but failsafe in favour of BSD)
        $<$<CONFIG:Release>:_FORTIFY_SOURCE=2>
    >
    $<$<CONFIG:Release>:NDEBUG>
    $<$<AND:$<BOOL:${JUST_THREAD_DEADLOCK_CHECK}>,$<CONFIG:Debug>>:_JUST_THREAD_DEADLOCK_CHECK>
    $<$<BOOL:${BOOST_DISABLE_ASSERTS}>:BOOST_DISABLE_ASSERTS>
)

target_compile_options(maidsafe_common
  PUBLIC
    $<$<BOOL:${MSVC}>:
        /W4      # Set warning level 4.
        /WX      # Treat warnings as errors.
        /MP7     # Enable multi-processor compilation (max 7).
        /EHsc    # Catches C++ exceptions only and tells the compiler to assume that extern C functions never throw a C++ exception.
        /TP      # Treat sources as C++.
        /wd4351  # Disable C4351 'new behavior: elements of array 'array' will be default initialized'.
                 # Unneeded for new code (only applies to code previously compiled with VS 2005).
        /wd4503  # Disable C4503 'decorated name length exceeded' caused by boost multi-index and signals2.
                 # Disabled as per advice at https://svn.boost.org/trac/boost/wiki/Guidelines/WarningsGuidelines.
        /wd4512  # Disable C4512 'assignment operator could not be generated' caused by boost signals2.
                 # Disabled as per advice at http://lists.boost.org/boost-users/2009/01/44368.php.
        /wd4913  # Disable C4913 'default built-in binary operator ',' used' caused by inclusion of boost/utility/result_of.hpp.
                 # Disabled due to boost bug https://svn.boost.org/trac/boost/ticket/7663.
        /wd4996  # Disable C4996 'Function call with parameters that may be unsafe' caused by boost signals2.
                 # Disabled as per advice at https://svn.boost.org/trac/boost/wiki/Guidelines/WarningsGuidelines.
        $<$<CONFIG:Release>:
            /O2  # Optimise code for maximum speed.  Implies the following:
                 #      Og (global optimisations)
                 #      Oi (replace some function calls with intrinsic functions),
                 #      Ot (favour fast code),
                 #      Oy (suppress creation of frame pointers on the call stack),
                 #      Ob2 (auto inline),
                 #      Gs (control stack probes),
                 #      GF (eliminate duplicate strings),
                 #      Gy (allows the compiler to package individual functions in the form of
                 #          packaged functions)
            /GL  # Whole program optimisation.
            /MD  # Use the multithread, dynamic version of the C run-time library.
        >
        $<$<CONFIG:Debug>:
            /Zi   # Produce a program database (.pdb) that contains type information and symbolic debugging information.
            /Od   # No optimizations in the program (speeds compilation).
            /RTC1 # Enables stack frame run-time error checking and checking for unintialised variables.
            /MDd  # Use the debug multithread, dynamic version of the C run-time library.
        >
        $<$<CONFIG:MinSizeRel>:
            /MD  # Use the multithread, dynamic version of the C run-time library.
        >
        $<$<CONFIG:RelWithDebInfo>:
            /O2  # Optimise code for maximum speed.
            /GL  # Whole program optimisation.
            /MD  # Use the multithread, dynamic version of the C run-time library.
            /Zi  # Produce a program database (.pdb) that contains type information and symbolic debugging information.
        >
    >
    $<$<BOOL:${UNIX}>:
        -std=c++11
        -pthread
        -W
        -Werror
        -Wall
        -Wextra
        -Wunused-parameter
        -Wno-system-headers
        -Wno-deprecated
        -Wwrite-strings
        -Wundef
        -Wuninitialized
        -Wparentheses
        -Wfloat-equal
        -Wstrict-overflow
        -Wstrict-overflow=5
        -Wredundant-decls
        -fPIC
        -pedantic
        -pedantic-errors
        $<$<CONFIG:Debug>:
            -O0
            -fno-inline
            -fno-eliminate-unused-debug-types
            -g3
            -ggdb
            ${CoverageFlags}
        >
        $<$<CONFIG:Release>:-O2>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
            ${LibCXX}
            $<$<CONFIG:Debug>:
                -fdiagnostics-format=clang
                -fdiagnostics-show-option
                -fdiagnostics-fixit-info
                -Wno-unused-command-line-argument
            >
        >
        $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
    >
)

