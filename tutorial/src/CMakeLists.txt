CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

FIND_PACKAGE(PkgConfig REQUIRED)

PROJECT(main CXX)

PKG_CHECK_MODULES(SYSTEMOC REQUIRED libsystemoc>=1.0)

#MESSAGE(STATUS "SYSTEMOC_STATIC_LDFLAGS: \"${SYSTEMOC_STATIC_LDFLAGS}\"")

UNSET(SYSTEMOC_STATIC_LIBRARIES CACHE)
UNSET(SYSTEMOC_STATIC_LDFLAGS_OTHER CACHE)
UNSET(SYSTEMOC_STATIC_LIBRARY_DIRS CACHE)
FOREACH(item ${SYSTEMOC_STATIC_LDFLAGS})
  IF(${item} MATCHES "^/.*/lib([^/]*)\\.(la|a|so)$")
    STRING(REGEX REPLACE "^(/.*)/lib([^/]*)\\.(la|a|so)$" "\\1" var_libdir  ${item})
    STRING(REGEX REPLACE "^(/.*)/lib([^/]*)\\.(la|a|so)$" "\\2" var_lib     ${item})
    STRING(REGEX REPLACE "^(/.*)/lib([^/]*)\\.(la|a|so)$" "\\3" var_libtype ${item})
    IF (${var_libtype} STREQUAL "la")
      IF(EXISTS "${var_libdir}/lib${var_lib}.so")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS "${var_libdir}")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    "${var_libdir}/lib${var_lib}.so")
      ELSEIF(EXISTS "${var_libdir}/.libs/lib${var_lib}.so")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS "${var_libdir}/.libs")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    "${var_libdir}/.libs/lib${var_lib}.so")
      ELSEIF(EXISTS "${var_libdir}/lib${var_lib}.a")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS "${var_libdir}")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    "${var_libdir}/lib${var_lib}.a")
      ELSEIF(EXISTS "${var_libdir}/.libs/lib${var_lib}.a")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS "${var_libdir}/.libs")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    "${var_libdir}/.libs/lib${var_lib}.a")
      ELSE()
        MESSAGE(FATAL_ERROR "Can't find library for ${var_libdir}/lib${var_lib}.${var_libtype}")
      ENDIF()
    ELSE()
      IF(EXISTS "${var_libdir}/lib${var_lib}.${var_libtype}")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS "${var_libdir}")
        LIST(APPEND SYSTEMOC_STATIC_LIBRARIES    "${var_libdir}/lib${var_lib}.${var_libtype}")
      ELSE()
        MESSAGE(FATAL_ERROR "Can't find library ${var_libdir}/lib${var_lib}.${var_libtype}")
      ENDIF()
    ENDIF()
    UNSET(var_libdir)
    UNSET(var_lib)
    UNSET(var_libtype)
  ELSEIF(${item} MATCHES "^-L[ \t]*(.*)$")
    STRING(REGEX REPLACE "^-L[ \t]*(.*)$" "\\1" var_libdir ${item})
    LIST(APPEND SYSTEMOC_STATIC_LIBRARY_DIRS ${var_libdir})
    UNSET(var_libdir)
  ELSEIF(${item} MATCHES "^-l[ \t]*(.*)$")
    STRING(REGEX REPLACE "^-l[ \t]*(.*)$" "\\1" var_lib ${item})
    LIST(APPEND SYSTEMOC_STATIC_LIBRARIES ${var_lib})
    UNSET(var_lib)
  ELSE()
    LIST(APPEND SYSTEMOC_STATIC_LDFLAGS_OTHER ${item})
  ENDIF()
ENDFOREACH()

#MESSAGE(STATUS "SYSTEMOC_STATIC_LIBRARIES:     \"${SYSTEMOC_STATIC_LIBRARIES}\"")
#MESSAGE(STATUS "SYSTEMOC_STATIC_LDFLAGS_OTHER: \"${SYSTEMOC_STATIC_LDFLAGS_OTHER}\"")
#MESSAGE(STATUS "SYSTEMOC_STATIC_LIBRARY_DIRS:  \"${SYSTEMOC_STATIC_LIBRARY_DIRS}\"")

ADD_LIBRARY(SysteMoC INTERFACE IMPORTED)
SET_TARGET_PROPERTIES(SysteMoC PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMOC_INCLUDE_DIRS}"
  INTERFACE_COMPILE_OPTIONS     "${SYSTEMOC_CFLAGS_OTHER}"
  INTERFACE_LINK_LIBRARIES      "${SYSTEMOC_STATIC_LIBRARIES};${SYSTEMOC_STATIC_LDFLAGS_OTHER}"
# INTERFACE_LINK_DIRECTORIES    "${SYSTEMOC_STATIC_LIBRARY_DIRS}" requires cmake >= 3.13
)

LINK_DIRECTORIES(${SYSTEMOC_STATIC_LIBRARY_DIRS})

ADD_EXECUTABLE(source-sink
  source-sink/cpp/vpc-source-sink.cpp)
TARGET_LINK_LIBRARIES(source-sink SysteMoC)

ADD_EXECUTABLE(source-sink-config-api
  source-sink-config-api/cpp/vpc-source-sink-config-api.cpp)
TARGET_LINK_LIBRARIES(source-sink-config-api SysteMoC)
