# - Find libavro
# Find the native LIBAVRO includes and library
#
#  LIBAVRO_INCLUDE_DIR - where to find avro.h, etc.
#  LIBAVRO_LIBRARIES   - List of libraries when using libavro.
#  LIBAVRO_FOUND       - True if libavro found.


if (LIBAVRO_INCLUDE_DIR)
  # Already in cache, be silent
  set(LIBAVRO_FIND_QUIETLY TRUE)
endif (LIBAVRO_INCLUDE_DIR)

find_path(LIBAVRO_INCLUDE_DIR avro.h
  /usr/local/include
  /usr/include
)

set(LIBAVRO_NAMES avro)
find_library(LIBAVRO_LIBRARY
  NAMES ${LIBAVRO_NAMES}
  PATHS /usr/lib /usr/local/lib
)

if (LIBAVRO_INCLUDE_DIR AND LIBAVRO_LIBRARY)
   set(LIBAVRO_FOUND TRUE)
    set( LIBAVRO_LIBRARIES ${LIBAVRO_LIBRARY} )
else (LIBAVRO_INCLUDE_DIR AND LIBAVRO_LIBRARY)
   set(LIBAVRO_FOUND FALSE)
   set( LIBAVRO_LIBRARIES )
endif (LIBAVRO_INCLUDE_DIR AND LIBAVRO_LIBRARY)

if (LIBAVRO_FOUND)
   if (NOT LIBAVRO_FIND_QUIETLY)
      message(STATUS "Found LIBAVRO: ${LIBAVRO_LIBRARY}")
   endif (NOT LIBAVRO_FIND_QUIETLY)
else (LIBAVRO_FOUND)
   if (LIBAVRO_FIND_REQUIRED)
      message(STATUS "Looked for libavro libraries named ${LIBAVROS_NAMES}.")
      message(FATAL_ERROR "Could NOT find libavro library")
   endif (LIBAVRO_FIND_REQUIRED)
endif (LIBAVRO_FOUND)

mark_as_advanced(
  LIBAVRO_LIBRARY
  LIBAVRO_INCLUDE_DIR
  )
