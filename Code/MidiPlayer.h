// *************************************************************************************************
//
// This class does all the real work for the game sound. It parses midi files, handles callback 
// events for playback, and distorts the song depending on the players performance.
//
// *************************************************************************************************

#ifndef __MIDI_PLAYER_H__
#define __MIDI_PLAYER_H__

#include <cstdlib>
#include <iostream>
#include "fluidsynth.h"
#include "MidiFileIn.h"
#include "Level.h"
#include <set>

#define SAMPLE float
#define DAMPING .8

using namespace std;
using namespace stk;

//struct for any number of midi events happening at same point in the song
struct EventBatch{
  vector<fluid_event_t *> events;
  double time;
};

class MidiPlayer
{
  public:
    MidiPlayer();
    ~MidiPlayer();

    bool init(string filename, Level *level);
    void clear();

    string getSongName();
    //Gets the names of the midi tracks
    vector<string> getTrackNames();

    //starts the midi song playing. Takes in the name of the track the user is playing.
    void playSong(string trackname);
    //stops the song from playing
    void stopSong();

    //Most important method here! Fills the current audio buffer with our midi song.
    void generate(SAMPLE *buffer, int nSamples);

    //Gets the current progress in the song as a double in the range 0 to 1 
    double getSongProgress();

    //Makes the synthesizer handle the current event.
    //Checks in with the game state to see if we should mess with the song.
    void playCurrEvent(short track);
    //asks the sequencer to schedule a callback for the next midi event.
    void scheduleNextEvent(short id);

  private:
    //helper methods
    //Fills the songEvents vector for future playback. Notifies game manager of each note onset.
    void parseEvents();
    //Translates the raw midi event bytes to a fluid event to be played by out sequencer.
    fluid_event_t *getFluidEvent(vector<unsigned char> &bytes);
    void setNextEvent(short track, fluid_event_t *evt);
    
    Level *level;
    fluid_synth_t *synth;
    fluid_sequencer_t *sequencer;

    MidiFileIn *mf;
    short ntracks;
    unsigned int start_time;
    double songLength;
    short synthSeqID;
    int gameTrack;

    map<short, short> changedNotes;
    map<string, int> trackNameToNo;
    vector<vector<EventBatch> >songEvents;
    vector<int> trackIndex;
    set<int> gameIDs;
    vector<short> callback_ids;
};

//fluid callback functions and data struct
struct CallbackData{
  MidiPlayer *sound;
  short track;
};
void seq_callback(unsigned int time, fluid_event_t* event, fluid_sequencer_t* seq, void* data);

#endif
