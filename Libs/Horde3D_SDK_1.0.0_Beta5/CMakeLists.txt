cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0003 NEW)

project(Horde3D)

# accumulate list of sources of extension modules
# this can not be done inside the Extensions folder since cmake can only pass variables to subdirectories, never upwards...
set(HORDE3D_EXTENSION_SOURCES)
set(HORDE3D_EXTENSION_INCLUDES)
set(HORDE3D_EXTENSION_INSTALLS)
set(HORDE3D_EXTENSION_INCLUDE_DIRS)
set(HORDE3D_EXTENSION_LIBS)


# terrain extension (check egExtensions.cpp if it's activated)
option(HORDE3D_BUILD_TERRAIN "Build the terrain extension into Horde3D" OFF)

# create egExtensions.cpp
CONFIGURE_FILE(Horde3D/Source/Horde3DEngine/egExtensions_auto_include.h.in ${CMAKE_BINARY_DIR}/egExtensions_auto_include.h)
CONFIGURE_FILE(Horde3D/Source/Horde3DEngine/egExtensions_auto_install.h.in ${CMAKE_BINARY_DIR}/egExtensions_auto_install.h)

add_subdirectory(Horde3D)