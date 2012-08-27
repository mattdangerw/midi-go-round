cmake_minimum_required(VERSION 2.7)
project (Midi-Go-Round)

find_package(ALSA REQUIRED)
find_library(X11_LIBRARY X11)

add_subdirectory(Libs)
include_directories(Libs/Horde3D_SDK_1.0.0_Beta5/Horde3D/Bindings/C++)
include_directories(Libs/glfw)
include_directories(Libs/glm)
include_directories(Libs/fluidsynth-1.1.6/include)

add_definitions(-D__LITTLE_ENDIAN__)
add_definitions(-D__LINUX_ALSASEQ__ -D__LINUX_ALSA__)

add_executable(Midi-Go-Round
  Code/GameManager.h
  Code/GameManager.cpp
  Code/GameStates.h
  Code/GameStates.cpp
  Code/MidiPlayer.h
  Code/MidiPlayer.cpp
  Code/Level.h
  Code/Level.cpp
  Code/MidiFileIn.h
  Code/MidiFileIn.cpp
  Code/RtAudio.h
  Code/RtAudio.cpp
  Code/RtError.h
  Code/Stk.h
  Code/Stk.cpp
  Code/main.cpp
  )

target_link_libraries(Midi-Go-Round Horde3D Horde3DUtils glfw libfluidsynth ${X11_LIBRARY})