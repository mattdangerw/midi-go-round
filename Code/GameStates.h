// *************************************************************************************************
//
// A class for every possible game state! An adhoc FSM.
// Each state must provide a method to handleInput, update the game each time step, and
// proceed to a new game state when the time comes.
//
// *************************************************************************************************

#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

#include "Level.h"
#include "MidiPlayer.h"
#include <Horde3D.h>
#include <vector> 
#include <sys/types.h>
#include <dirent.h>

using namespace std;

struct GameData{
    Level *level;
    MidiPlayer *player;
    H3DRes font;
    H3DNode cam;
};

//Forward declaration. Exits the program.
void quit();

//abstract game state
class GameState
{
  public:
    GameState( GameData *gd );
    virtual ~GameState () {};
    
    virtual GameState *checkForChange() { return NULL; };

    virtual void update( float time );

    virtual void handleMouseInput( bool down, float x, float y ) {};

    GameData *gd;
    float entered, timeInState;
};

//generic menu state. subclasses need to define state transitions and menu items
class MenuState : public GameState
{
  public:
    MenuState( GameData *gd );
    
    virtual GameState *checkForChange() { return NULL; };

    void update( float time );

    void handleMouseInput( bool down, float x, float y );

    bool doneSelecting();

    vector<string> options;
    string title;
    bool backOption;

    int page;
    bool wasDown;
    float mouseX, mouseY;
    string selected, highlighted;
    vector<string> pageOptions;

    //helpers
    void showPage();
    void updatePageOptions();
};

class MainMenuState : public MenuState
{
  public:
    MainMenuState( GameData *gd );
    
    GameState *checkForChange();
};

class SongMenuState : public MenuState
{
  public:
    SongMenuState( GameData *gd );
    
    GameState *checkForChange();
};

class TrackMenuState : public MenuState
{
  public:
    TrackMenuState( GameData *gd );
    
    GameState *checkForChange();
};

class PlayingState : public GameState
{
  public:
    PlayingState( GameData *gd );
    
    GameState *checkForChange();

    void update( float time );

    void handleMouseInput( bool down, float x, float y );

    double progress;

};

//Generic state to display a message to the user
class MessageState : public GameState
{
  public:
    MessageState( GameData *gd );
    
    virtual GameState *checkForChange() { return NULL; };

    void update( float time );

    void handleMouseInput( bool down, float x, float y );

    bool wasDown, isDown, centered;
    float mouseX, mouseY;
    vector<string> text;

};

//explains the games. briefly
class HowToState : public MessageState
{
  public:
    HowToState( GameData *gd );

    GameState *checkForChange();
};

//warns when single track midi is selected
class BadMidiState : public MessageState
{
  public:
    BadMidiState( GameData *gd );

    GameState *checkForChange();
};

//shows the final score. and stuff
class FinalScoreState : public GameState
{
  public:
    FinalScoreState( GameData *gd );

    GameState *checkForChange();

    void update( float time );

    void handleMouseInput( bool down, float x, float y );

    bool wasDown, isDown;
    string scoreText;
};

#endif