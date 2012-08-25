Midi-Go-Round
=============

About This Project
------------------

A small midi based game for linux. Written for Music, Computing, and Design I at CCRMA.

How To Build
------------

This game uses ALSA for audio and needs the ALSA libraries and headers. If you don't have them already try:

    sudo apt-get install libasound2-dev

Next up get cmake, from their [website](cmake.org), or from the package manager again:

    sudo apt-get install cmake

Then it will hopefully be as easy as going to the home directory of the project and running:

    cmake .
    make

There may be additional dependencies you still need (gcc, pthreads...). Watch the output from invoking cmake.

Once it builds, your good to go!

    ./Midi-Go-Round

Libraries Used
--------------

- [Horde3D](www.horde3d.org)
- [GLFW](www.glfw.org)
- [FluidSynth](www.fluidsynth.org)
- [RTAudio](www.music.mcgill.ca/~gary/rtaudio/)