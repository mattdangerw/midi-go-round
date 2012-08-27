// *************************************************************************************************
//
// The GameManager class sets up all the Hoard basics to begin displaying the scene,
// handles the basic game state (menus, in game, etc), and handles user input.
//
// *************************************************************************************************

#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "Level.h"
#include "MidiPlayer.h"
#include "GameStates.h"
#include <Horde3D.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector> 
#include <map>

using namespace std;

class GameManager
{
  public:
    GameManager();
  	
    bool init(MidiPlayer *player);

    //stop drawing things! tells horde to quit
    void stop();

    //Updates position, state of everything in the scene
    void mainLoop( float time );

    void updateMouse( bool down, float x, float y );

    void resize( int width, int height );

    void screenshot();

  private:
    GameState *state;
    MidiPlayer *player;
    Level *level;
    
    bool debugViewMode, wireframeMode, screenshotNextFrame;
  	
    std::string contentDir;
    // Engine objects
    H3DRes hdrPipeRes, forwardPipeRes, fontRes;
    H3DNode cam;
};

#endif

