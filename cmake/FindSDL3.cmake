if(NOT PREFER_BUNDLED_LIBS)
  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
  find_package(SDL3)
  set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})
  if(SDL3_FOUND)
    set(SDL3_BUNDLED OFF)
    set(SDL3_DEP)
  endif()
endif()

if(NOT CMAKE_CROSSCOMPILING)
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_SDL3 sdl3)
endif()

set_extra_dirs_lib(SDL3 sdl3)
find_library(SDL3_LIBRARY
  NAMES SDL3
  HINTS ${HINTS_SDL3_LIBDIR} ${PC_SDL3_LIBDIR} ${PC_SDL3_LIBRARY_DIRS}
  PATHS ${PATHS_SDL3_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)
set(CMAKE_FIND_FRAMEWORK FIRST)
set_extra_dirs_include(SDL3 sdl3 "${SDL3_LIBRARY}")
# Looking for 'SDL.h' directly might accidentally find a SDL 1 instead of SDL 3
# installation. Look for a header file only present in SDL 3 instead.
find_path(SDL3_INCLUDEDIR SDL3/SDL_assert.h
  PATH_SUFFIXES SDL3
  HINTS ${HINTS_SDL3_INCLUDEDIR} ${PC_SDL3_INCLUDEDIR} ${PC_SDL3_INCLUDE_DIRS}
  PATHS ${PATHS_SDL3_INCLUDEDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL3 DEFAULT_MSG SDL3_LIBRARY SDL3_INCLUDEDIR)

mark_as_advanced(SDL3_LIBRARY SDL3_INCLUDEDIR)

if(SDL3_FOUND)
  set(SDL3_LIBRARIES ${SDL3_LIBRARY})
  set(SDL3_INCLUDE_DIRS ${SDL3_INCLUDEDIR})

  is_bundled(SDL3_BUNDLED "${SDL3_LIBRARY}")
  if(SDL3_BUNDLED AND TARGET_OS STREQUAL "windows")
    set(SDL3_COPY_FILES "${EXTRA_SDL3_LIBDIR}/SDL3.dll")
  else()
    set(SDL3_COPY_FILES)
  endif()
endif()
