// *************************************************************************************************
//
// The level class sets up all objects in the level and keeps and them animated.
//
// *************************************************************************************************

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <Horde3D.h>
#include <string>
#include <cstdlib>
#include <vector> 
#include <map>

using namespace std;

//a little struct for each floating note we will have in the level
struct Note{
  int group;
  int note;
  double onset;
};

class Level
{
 public:
  Level();
  ~Level();
  
  void init(H3DRes particleSysRes, H3DRes pinwheelRes, H3DRes noteRes,
    H3DRes transMatRes, H3DRes starMatRes );

  void clear();

  //updates the level during normal gameplay
  void update(float time, double songProgress);
  //updates the level with a passive spin only. no notes
  void spin(float time);

  //use to move the player around, when not in game
  void placePlayer( const float *mat );
  void lockPlayer();
  void unlockPlayer();

  void handleMouseInput(float mx);

  //These four methods allow the midi player to interact with the level
  void setBeat( double beat );
  void setNote( int id, int note, double onset );
  void finalizeNotes();
  //Checks if the note with the given ID was hit by the player.
  bool checkNote(int id);

  int getScore();
  //gets the current "powerup level"
  //0 1 or 2
  int getPowerup();

  //attaches camera to the top of the level, use when in game
  void attachCamera(H3DNode cam);

 private:
  //helper methods
  void addTargetLine(H3DRes transMatRes);
  void addStars();
  void removeHits();
  void addNoteObj( int id, float degrees, float dx);

  float lastFrameTime, lastHit;

  float playerX;

  double beat;

  int noteMax;
  int noteMin;
  
  int score;
  int streak;
    
  // Engine objects
  H3DRes noteRes, starMatRes;
  H3DNode playerAttach, player, particleSys, turnNode, pinwheel, targetLine;

  vector<Note> notes;
  map<int,vector<int> > groupMap;
  vector<H3DRes> attachPoints;
  vector<H3DRes> noteNodes;

  vector<bool> removed;
  vector<int> toRemove;
};

#endif

