#include "MidiPlayer.h"
#include <sstream>

#define DAMPING .65

MidiPlayer::MidiPlayer(){
  gameTrack = 1;
  fluid_settings_t* settings = new_fluid_settings();
  synth = new_fluid_synth(settings);
  fluid_synth_sfload(synth, "Content/sounds.sf2", 1);
  sequencer = new_fluid_sequencer2(0);
  synthSeqID = fluid_sequencer_register_fluidsynth(sequencer, synth);
}

MidiPlayer::~MidiPlayer(){
  delete_fluid_sequencer(sequencer);
  delete_fluid_synth(synth);
}

bool MidiPlayer::init(string filename, Level *l){
  mf = new MidiFileIn(filename);

  if(mf->getFileFormat()!=1){
    exit(0);
  }

  level = l;
  ntracks = mf->getNumberOfTracks();
  return true;
}

void MidiPlayer::clear(){
  delete mf;
  gameTrack = 1;

  for(int i = 0; i < callback_ids.size(); i++){
    fluid_sequencer_unregister_client(sequencer, callback_ids[i]);
  }
  delete_fluid_sequencer(sequencer);
  sequencer = new_fluid_sequencer2(0);
  synthSeqID = fluid_sequencer_register_fluidsynth(sequencer, synth);

  changedNotes.clear();
  trackNameToNo.clear();
  songEvents.clear();
  trackIndex.clear();
  gameIDs.clear();
  callback_ids.clear();
}

string MidiPlayer::getSongName(){
  string name = "";
  while(true){
    vector<unsigned char> bytes;
    long ticks = mf->getNextEvent(&bytes, 0);
    if(bytes.size() == 0) break;
    if(bytes[0] == 255){
      if(bytes[1] == 3){
        for(int i = 3; i < bytes.size(); i++){
          name += bytes[i];
        }
        break;
      }
    }
  }
  mf->rewindTrack();
  return name;
}

vector<string> MidiPlayer::getTrackNames(){
  vector<string> trackNames;
  for(int i = 1; i < ntracks; i++){
    stringstream ss;
    ss << "Untitled Track " << i;
    string trackName = ss.str();
    while(true){
      vector<unsigned char> bytes;
      long ticks = mf->getNextEvent(&bytes, i);
      if(bytes.size() == 0) break;
      if(bytes[0] == 255){
        if(bytes.size() > 2 && bytes[1] == 3){
          trackName = "";
          for(int j = 3; j < bytes.size(); j++){
            trackName += bytes[j];
          }
        }
      }
      if(getFluidEvent(bytes) != NULL){
        trackNameToNo[trackName] = i;
        trackNames.push_back(trackName);
        break;
      }
    }
  }
  for(int i = 0; i < ntracks; i++){
    mf->rewindTrack();
  }
  return trackNames;
}

void MidiPlayer::parseSong(string trackname){
  //parse the events in the midi
  gameTrack = trackNameToNo[trackname];
  parseEvents();
}

void MidiPlayer::playSong(){
  for(int i = 0; i < ntracks; i++){
    CallbackData *dat = new CallbackData;
    dat->sound = this;
    dat->track = i;
    int id = fluid_sequencer_register_client(sequencer, "track" + i, seq_callback, dat);
    callback_ids.push_back(id);
  }

  //start up each track
  start_time = fluid_sequencer_get_tick(sequencer);
  for(int i = 0; i < ntracks; i++){
    trackIndex.push_back(-1);
    scheduleNextEvent(i);
  }
}

void MidiPlayer::stopSong(){
}

void MidiPlayer::generate(SAMPLE *buffer, int nSamples){
  fluid_synth_write_float( synth, nSamples, buffer, 0, 2, buffer, 1, 2 );
}

double MidiPlayer::getSongProgress(){
  return (fluid_sequencer_get_tick(sequencer) - start_time)/songLength;
}

//Each time this callback is called, it's time to play the current event and schedule a callback for the next one.
void seq_callback(unsigned int time, fluid_event_t* event, fluid_sequencer_t* seq, void* data){
  if( fluid_event_get_type(event) == FLUID_SEQ_UNREGISTERING) {
    delete (CallbackData *)data;
    return;
  }
  MidiPlayer *gs = ((CallbackData *)data)->sound;
  short track = ((CallbackData *)data)->track;
  gs->playCurrEvent(track);
  gs->scheduleNextEvent(track);
}

void MidiPlayer::playCurrEvent(short track){
  vector<fluid_event_t *> *events = &songEvents[track][trackIndex[track]].events;
  int eventID = trackIndex[track];
  bool checked = false;
  bool bad = false;
  for(int i = 0; i < events->size(); i++){
    fluid_event_t *evt = events->at(i);
    int evtType = fluid_event_get_type(evt);
    if(track == gameTrack){
      if(evtType == FLUID_SEQ_NOTEON){
        short note = fluid_event_get_key(evt);
        changedNotes.erase(note);
      }
      if(gameIDs.count(eventID) > 0 && evtType == FLUID_SEQ_NOTEON){
        if(!checked){
          bad = !level->checkNote(eventID);
          checked = true;
        }
        if(bad){
          int channel = fluid_event_get_channel(evt);
          short oldnote = fluid_event_get_key(evt);
          short err = rand()%4 -2;
          if(err >= 0) err += 1;
          short note = oldnote + err;
          changedNotes[oldnote] = note;
          short vel = fluid_event_get_velocity(evt);
          //evt = new_fluid_event();
          fluid_event_noteon(evt, channel, note, vel);
        }
      }
      if(evtType == FLUID_SEQ_NOTEOFF){
        short note = fluid_event_get_key(evt);
        if(changedNotes.count(note) > 0){
          short newnote = changedNotes[note];
          changedNotes.erase(note);
          int channel = fluid_event_get_channel(evt);
          fluid_event_noteoff(evt, channel, newnote);
        }
      }
    }
    else{
      if(evtType == FLUID_SEQ_NOTEON){
        int channel = fluid_event_get_channel(evt);
        short note = fluid_event_get_key(evt);
        short vel = fluid_event_get_velocity(evt);
        fluid_event_noteon(evt, channel, note, short(vel * DAMPING));
      }
      else if(evtType == FLUID_SEQ_CHANNELPRESSURE){
        int channel = fluid_event_get_channel(evt);
        short vel = fluid_event_get_velocity(evt);
        fluid_event_channel_pressure(evt, channel, short(vel * DAMPING));
      }
      else if(evtType == FLUID_SEQ_NOTEOFF){
        short note = fluid_event_get_key(evt);
        int channel = fluid_event_get_channel(evt);
        fluid_event_noteoff(evt, channel, note);
      }
    }
    if(evt)
      fluid_sequencer_send_now(sequencer, evt);
  }
}

void MidiPlayer::scheduleNextEvent(short track){
  if(track == 0) return;
  int index = ++trackIndex[track];
  if(index >= songEvents[track].size()) return;
  
  fluid_event_t *callback_evt = new_fluid_event();
  fluid_event_set_source(callback_evt, -1);
  fluid_event_set_dest(callback_evt, callback_ids[track]);
  fluid_event_timer(callback_evt, NULL);
  fluid_sequencer_send_at(sequencer, callback_evt, (unsigned int)(start_time + songEvents[track][index].time), 1);
  delete_fluid_event(callback_evt);
}

void MidiPlayer::parseEvents(){
  songEvents.push_back(vector<EventBatch>());
  //minimum song length is one second
  songLength = 1000;
  vector<int> onsetNotes;
  vector<double> onsetTimes;
  vector<int>onsetIDs;
  double lastTime = 500;
  int lastID = -1;
  map<int, int> tickCounter;
  double dtick = 0;

  //loop trough every track
  for(int i = 1; i < ntracks; i++){
    double eventTime = 0;
    int eventID = 0;
    EventBatch batch;
    batch.time = eventTime;
    vector<EventBatch> trackEvents;

    //loop through events in each track.
    //Most of this code makes sure every event happening at the same time gets grouped in a batch.
    while(true){
      vector<unsigned char> bytes;
      //get time passed since last event
      long ticks = mf->getNextMidiEvent(&bytes, i);

      //end of track!
      if(bytes.size() == 0) break;
      
      //Time has passed. Push back old batch of events, start a new one.
      if(ticks > 0){  
        if(batch.events.size() > 0){
          trackEvents.push_back(batch);
          eventID++;
          batch.events.clear();
        }
        dtick = mf->getTickSeconds() * 1000;
        if(ticks*dtick > 100)tickCounter[ticks]++;
        eventTime += dtick * ticks;
        batch.time = eventTime;
      }

      //no time has passed. Add to event batch
      fluid_event_t *evt = getFluidEvent(bytes);
      if(evt != NULL){
        batch.events.push_back(evt);

        if(i == gameTrack && fluid_event_get_type(evt) == FLUID_SEQ_NOTEON && 
           (eventTime - lastTime > 200 || lastID == eventID)){
          lastTime = eventTime;
          lastID = eventID;
          gameIDs.insert(eventID);
          onsetNotes.push_back(bytes[1]);
          onsetTimes.push_back(eventTime);
          onsetIDs.push_back(eventID);
        }
      }
    }

    //push the last event batch onto the track.
    if(batch.events.size() > 0){
      trackEvents.push_back(batch);
    }

    songEvents.push_back(trackEvents);
    //update song length
    if(eventTime > songLength){
      songLength = eventTime;
    }
  }

  int commonTickInterval = 100, mostTimes = 0;
  for(map<int, int>::iterator itr = tickCounter.begin(); itr != tickCounter.end(); itr++){
    if(itr->second > mostTimes){
      commonTickInterval = itr->first;
      mostTimes = itr->second;
    }
  }
  double beat = dtick * commonTickInterval;
  while( beat < 500 ) beat *= 2;

  //Tell the game manager where to place the notes!
  for(int i = 0; i < onsetNotes.size(); i++){
    level->setNote(onsetIDs[i], onsetNotes[i], onsetTimes[i]/songLength);
  }
  level->setBeat( beat/songLength );
  level->finalizeNotes();
}

fluid_event_t *MidiPlayer::getFluidEvent(vector<unsigned char> &bytes){
  unsigned int code = bytes[0];
  fluid_event_t *evt = new_fluid_event();
  fluid_event_set_source(evt, -1);
  fluid_event_set_dest(evt, synthSeqID);

  //noteoff
  if(code >=128 && code <= 143){
    unsigned char chan = code - 128;
    fluid_event_noteoff(evt, chan, bytes[1]);
    return evt;
  }
  
  //noteon
  else if(code >= 144 && code <= 159){
    unsigned char chan = code - 144;
    if(int(bytes[2]) == 0){
      fluid_event_noteoff(evt, chan, bytes[1]);
      return evt;
    }
    fluid_event_noteon(evt, chan, bytes[1], bytes[2]);
    return evt;
  }
  
  //polyphonic aftertouch
  //fluidsynth doesn't have this so we'll just treat it as channel wide
  else if(code >= 160 && code <= 175){
    unsigned char chan = code - 160;
    fluid_event_channel_pressure(evt, chan, bytes[2]);
    return evt;
  }
  
  //control change
  else if(code >= 176 && code <= 191){
    unsigned char chan = code - 176;
    fluid_event_control_change(evt, chan, bytes[1], bytes[2]);
    return evt;
  }
  
  //program change
  else if(code >= 192 && code <= 207){
    unsigned char chan = code - 192;
    fluid_event_program_change(evt, chan, bytes[1]);
    return evt;
  }

  //channel aftertouch
  else if(code >= 208 && code <= 223){
    unsigned char chan = code - 208;
    fluid_event_channel_pressure(evt, chan, bytes[1]);
    return evt;
  }
  
  //pitch change
  else if(code >= 224 && code <= 239){
    unsigned char chan = code - 224;
    int lsb = (int)bytes[1];
    int msb = (int)bytes[2];
    fluid_event_pitch_bend(evt, chan, msb*128 + lsb);
    return evt;
  }

  //we don't know this midi code
  return NULL;
}