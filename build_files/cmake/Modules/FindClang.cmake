# SPDX-FileCopyrightText: 2011 Blender Authors
#
# SPDX-License-Identifier: BSD-3-Clause

# - Find Clang library
# Find the native Clang includes and library
# This module defines
#  CLANG_INCLUDE_DIRS, where to find AST/AST.h, Set when
#                            CLANG_INCLUDE_DIR is found.
#  CLANG_LIBRARIES, libraries to link against to use Clang.
#  CLANG_ROOT_DIR, The base directory to search for Clang.
#                        This can also be an environment variable.
#  CLANG_FOUND, If false, do not try to use Clang.

# If `CLANG_ROOT_DIR` was defined in the environment, use it.
if(DEFINED CLANG_ROOT_DIR)
  # Pass.
elseif(DEFINED ENV{CLANG_ROOT_DIR})
  set(CLANG_ROOT_DIR $ENV{CLANG_ROOT_DIR})
else()
  set(CLANG_ROOT_DIR "")
endif()

if(NOT LLVM_ROOT_DIR)
  if(DEFINED LLVM_VERSION)
    message(running llvm-config-${LLVM_VERSION})
    find_program(LLVM_CONFIG llvm-config-${LLVM_VERSION})
  endif()
  if(NOT LLVM_CONFIG)
    find_program(LLVM_CONFIG llvm-config)
  endif()

  execute_process(COMMAND ${LLVM_CONFIG} --prefix
          OUTPUT_VARIABLE LLVM_ROOT_DIR
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_ROOT_DIR ${LLVM_ROOT_DIR} CACHE PATH "Path to the LLVM installation")
endif()

set(_clang_SEARCH_DIRS
  ${CLANG_ROOT_DIR}
  ${LLVM_ROOT_DIR}
  /opt/lib/clang
)

find_path(CLANG_INCLUDE_DIR
  NAMES
    AST/AST.h
  HINTS
    ${_clang_SEARCH_DIRS}
  PATH_SUFFIXES
    include
    include/clang
)


set(_clang_FIND_COMPONENTS
  clangDependencyScanning
  clangDynamicASTMatchers
  clangFrontendTool
  clangStaticAnalyzerFrontend
  clangHandleCXX
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangToolingASTDiff
  clangToolingRefactoring
  clangToolingSyntax
  clangARCMigrate
  clangCodeGen
  clangCrossTU
  clangIndex
  clangTooling
  clangFormat
  clangToolingInclusions
  clangRewriteFrontend
  clangFrontend
  clangSerialization
  clangDriver
  clangToolingCore
  clangParse
  clangRewrite
  clangSema
  clangEdit
  clangAnalysis
  clangASTMatchers
  clangAST
  clangLex
  clangBasic
)

set(_clang_LIBRARIES)
foreach(COMPONENT ${_clang_FIND_COMPONENTS})
  string(TOUPPER ${COMPONENT} UPPERCOMPONENT)

  find_library(CLANG_${UPPERCOMPONENT}_LIBRARY
    NAMES
      ${COMPONENT}
    HINTS
      ${_clang_SEARCH_DIRS}
    PATH_SUFFIXES
      lib64 lib
  )
  if(CLANG_${UPPERCOMPONENT}_LIBRARY)
    list(APPEND _clang_LIBRARIES "${CLANG_${UPPERCOMPONENT}_LIBRARY}")
  endif()
endforeach()


# Handle the QUIETLY and REQUIRED arguments and set CLANG_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Clang DEFAULT_MSG
  _clang_LIBRARIES CLANG_INCLUDE_DIR)

if(CLANG_FOUND)
  set(CLANG_LIBRARIES ${_clang_LIBRARIES})
  set(CLANG_INCLUDE_DIRS ${CLANG_INCLUDE_DIR})
endif()

mark_as_advanced(
  CLANG_INCLUDE_DIR
)

foreach(COMPONENT ${_clang_FIND_COMPONENTS})
  string(TOUPPER ${COMPONENT} UPPERCOMPONENT)
  mark_as_advanced(CLANG_${UPPERCOMPONENT}_LIBRARY)
endforeach()

unset(_clang_SEARCH_DIRS)
unset(_clang_FIND_COMPONENTS)
unset(_clang_LIBRARIES)
