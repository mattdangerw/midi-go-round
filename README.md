Midi-Go-Round
=============

About This Project
------------------

Midi-Go-Round is a simple rhythm game using midi files. The game was written originally for Music, Computing, and Design I a CCRMA course at Stanford.

Gameplay is simple, the goal is to hit all the "notes" in a level to play a certain instrument in a midi song. Midi files, though they don't always sound the best, completely reveal the song structure. This makes it possible to build levels from arbitrary midi files and to distort the song as its being played, depending on user performance.

This was also an excuse to learn about a very compact game engine called Horde3D. It has a flexible rendering pipeline and minimalist design, which drew me to it. Not sure I would suggest it in the end--there's no support and few examples.

How To Build
------------

This game uses ALSA for audio and needs the ALSA libraries and headers. If you don't have them already try:

    sudo apt-get install libasound2-dev

Next up get cmake from their website, www.cmake.org, or from the package manager again:

    sudo apt-get install cmake

Then it will hopefully be as easy as going to the home directory of the project and running:

    cmake .
    make

There may be additional dependencies you still need (gcc, pthreads...). Watch the output from invoking cmake.

Once it builds, your good to go!

    ./Midi-Go-Round

How To Play
-----------

To start you select a midi song and track (a single instrument in the song) to play from the first few menus. The track you select will become a level of notes to hit. Vocal tracks tend to be the best to play, but often the tracks are poorly labeled in the Midi file. For the midi songs included here, the vocal tracks are:
- beatles: Lead Organ 3
- here comes your man: Chant
- house of the rising sun: Melody
- sedated: Voice

You can place any midi file you'd like in the Midis directory, and it will show up in game. There are two types of midi files commonly used, one with a separate track for each instrument and one with all the instruments thrown together. Only type 1 midis, with separate tracks, will work with the game.

There's a bug in the game making the audio wig out occasionally after a few song have been played :( I'd suggest restarting between songs for now.

Press s for a screenshot. Escape to quit.

Libraries Used
--------------

- Horde3D: a very compact rendering engine. www.horde3d.org
- GLFW: a library for input and window management with OpenGL. www.glfw.org
- GLM: a math library with GLSL syntax. http://glm.g-truc.net/
- FluidSynth: a midi synthesizer and sequencer. www.fluidsynth.org
- RTAudio: a library for real time sound rendering. www.music.mcgill.ca/~gary/rtaudio/