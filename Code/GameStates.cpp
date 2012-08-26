#include "GameStates.h"
#include <Horde3DUtils.h>
#include <iostream>
#include <algorithm>
#include <strings.h>
#include <cmath>

#define FONT_SIZE .056f
#define OPTIONS_PER_PAGE 15

bool iequals(const string& a, const string& b)
{
  return strcasecmp(a.c_str(), b.c_str()) < 0;
}

GameState::GameState( GameData *gd ) {
  this->gd = gd;
  entered = -1;
  timeInState = 0;
}

void GameState::update( float time ) {
  if( entered < 0 ) {
    entered = time;
  }
  timeInState = time - entered;
}

MenuState::MenuState( GameData *gd ) : GameState( gd ) {
  mouseX = -1;
  mouseY = -1;
  wasDown = false;
  title = "";
  selected = "";
  highlighted = "";
  page = 0;
  backOption = false;
}

void MenuState::update( float time ) {
  GameState::update( time );
  h3dutShowText( title.c_str(), 0.03f, 0.03f, FONT_SIZE, .9, .9, .9, gd->font );
  showPage();

  //calculate location of object at mouse point 4 units from camera
  //and place the particle emitter there
  if(mouseX > 0) {
    float orig[3];
    float dir[3];
    h3dutPickRay( gd->cam, mouseX, 1 - mouseY, orig, orig + 1, orig + 2, dir, dir + 1, dir + 2 );
    float dirLen = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );
    float mul = 15 / dirLen;
    gd->level->placePlayer(orig[0] + dir[0] * mul,
      orig[1] + dir[1] * mul, orig[2] + dir[2] * mul);
  }

  gd->level->spin( time );
}

void MenuState::handleMouseInput( bool down, float x, float y ) {
  if(timeInState > .1 && !down && wasDown && highlighted.length() > 0) {
    if(page > 0 && highlighted.compare("<<< Back") == 0) {
      page--;
      updatePageOptions();
    }
    else if(highlighted.compare("Next >>>") == 0) {
      page++;
      updatePageOptions();
    }
    else selected = highlighted;
  }
  mouseY = y;
  mouseX = x;
  wasDown = down;
}

void MenuState::showPage() {
  if(pageOptions.empty()) updatePageOptions();
  float textY = 0.1f;
  highlighted = "";
  for(int i = 0; i < pageOptions.size(); i++){
    if( mouseY >= textY && mouseY < (textY + 0.075f) ) {
      h3dutShowText( pageOptions[i].c_str(), 0.06, textY, FONT_SIZE, .9, .2, .2, gd->font );
      highlighted = pageOptions[i];
      textY+=.075;
    }
    else {
      h3dutShowText( pageOptions[i].c_str(), 0.06, textY, FONT_SIZE * .8f, .2, .9, .2, gd->font );
      textY+=.06f;
    }
  }
}

//figure out what options to display on this menu "page"
void MenuState::updatePageOptions() {
  pageOptions.clear();
  //add a back option to previous page or menu if necessary
  if( page > 0 || backOption ) {
    pageOptions.push_back("<<< Back");
  }

  //find the index to start in our full list of options
  //based on the page
  int optionIndex = 0;
  if( page > 0 ) {
    optionIndex = (OPTIONS_PER_PAGE - 2) * page;
    if(!backOption) optionIndex++;
  }

  //populate this page's list
  while(pageOptions.size() < OPTIONS_PER_PAGE - 1) {
    if( optionIndex == options.size() ) break;
    pageOptions.push_back(options[optionIndex]);
    optionIndex++;
  }

  //add a next option if necessary
  if( optionIndex < options.size() ) {
    pageOptions.push_back("Next >>>");
  }
}

bool MenuState::doneSelecting() {
  return (selected.length() > 0);
}

MainMenuState::MainMenuState( GameData *gd ) : MenuState( gd ) {
  title = "Menu";
  options.push_back("Play");
  options.push_back("How To");
  options.push_back("Quit");
  h3dSetNodeParent( gd->cam, H3DRootNode );
  h3dSetNodeTransform( gd->cam, 600, 0, 0, 0, 90, 0, 1, 1, 1 );
  gd->level->unlockPlayer();
}

GameState *MainMenuState::checkForChange() { 
  if( doneSelecting() ) {
    if( selected.compare("Play") == 0) {
      return new SongMenuState( gd );
    }
    if( selected.compare("How To") == 0 ) {
      return new HowToState( gd );
    }
    if( selected.compare("Quit") == 0 ) {
      quit();
    }
  }
  return NULL;
}

SongMenuState::SongMenuState( GameData *gd ) : MenuState( gd ) {
  title = "Select A Song";
  DIR *dir = opendir("Midis");
  dirent *entry;
  backOption = true;
  while((entry = readdir(dir)) != NULL) {
    string fileName = entry->d_name;
    if(fileName[0] != '.') {
      options.push_back(entry->d_name);
    }
  }
  sort(options.begin(), options.end(), iequals);
}

GameState *SongMenuState::checkForChange() { 
  if( doneSelecting() ) {
    if(selected.compare("<<< Back") == 0 ) {
      return new MainMenuState( gd );
    }
    if(gd->player->init("Midis/" + selected, gd->level)) {
      return new TrackMenuState( gd );
    }
    else {
      return new BadMidiState( gd );
    }
  }
  return NULL;
}

TrackMenuState::TrackMenuState( GameData *gd ) : MenuState( gd ) {
  title = gd->player->getSongName() + " - Select A Track";
  options = gd->player->getTrackNames();
  backOption = true;
}

GameState *TrackMenuState::checkForChange() { 
  if( doneSelecting() ) {
    if(selected.compare("<<< Back") == 0 ) {
      return new SongMenuState( gd );
    }
    gd->player->parseSong(selected);
    gd->player->playSong();
    gd->level->lockPlayer();
    return new PlayingState( gd );
  }
  return NULL;
}

PlayingState::PlayingState( GameData *gd ) : GameState( gd ) { 
  gd->level->attachCamera(gd->cam);
}

GameState *PlayingState::checkForChange() {
  if(progress > 1.0) return new FinalScoreState( gd );
  return NULL;
}

void PlayingState::update( float time ) { 
  GameState::update( time );

  progress = gd->player->getSongProgress();
  gd->level->update( time, progress );
  
  //h3dutShowText( songName.c_str(), 0.03f, 0.03f, FONT_SIZE, .9, .9, .9, gd->font );
  stringstream ss;
  ss << "Score: ";
  ss << gd->level->getScore();
  string scoreText = ss.str();
  int powerup = gd->level->getPowerup();
  if(powerup == 2){
    h3dutShowText( scoreText.c_str(), 1.3f, 0.03f, FONT_SIZE, .9, .2, .2, gd->font );
  }
  else if(powerup == 1){
    h3dutShowText( scoreText.c_str(), 1.3f, 0.03f, FONT_SIZE, .2, .9, .2, gd->font );
  }
  else{
    h3dutShowText( scoreText.c_str(), 1.3f, 0.03f, FONT_SIZE, .9, .9, .9, gd->font );
  }
}

void PlayingState::handleMouseInput( bool down, float x, float y ){ 
  gd->level->handleMouseInput(x);
}

MessageState::MessageState( GameData *gd ) : GameState( gd ) {
  isDown = false;
  wasDown = false;
  centered = false;
}

void MessageState::update( float time ) {
  GameState::update( time );

  float verticalSpace = text.size() * FONT_SIZE + .02 * (text.size() - 1);
  float textY = .5 - verticalSpace/2;
  for(int i = 0; i < text.size(); i++) {
    if(centered) {
      float textX = .883 - text[i].size() * FONT_SIZE / 4;
      h3dutShowText( text[i].c_str(), textX, textY, FONT_SIZE, 1.0, 1.0, 1.0, gd->font ); 
    }
    else {
      h3dutShowText( text[i].c_str(), 0.03f, textY, FONT_SIZE, 1.0, 1.0, 1.0, gd->font );
    }
    textY += FONT_SIZE + .02;
  }

  gd->level->spin( time );
}

void MessageState::handleMouseInput( bool down, float x, float y ){ 
  wasDown = isDown;
  isDown = down;
}

HowToState::HowToState( GameData *gd ) : MessageState( gd ) {
  text.push_back("Someday");
  text.push_back("These will be instructions on how to play the game.");
}

GameState *HowToState::checkForChange() { 
  if( isDown && ! wasDown ) {
    return new MainMenuState( gd );
  }
  return NULL;
}

BadMidiState::BadMidiState( GameData *gd ) : MessageState( gd ) {
  text.push_back("We don't support this midi file format!");
  text.push_back("Use the one with multiple track so we can use the tracks in game.");
}

GameState *BadMidiState::checkForChange() { 
  if( isDown && ! wasDown ) {
    return new SongMenuState( gd );
  }
  return NULL;
}

FinalScoreState::FinalScoreState( GameData *gd ) : GameState( gd ) {
  isDown = false;
  wasDown = false;

  stringstream ss;
  ss << "Score: ";
  ss << gd->level->getScore();
  scoreText = ss.str();
}

void FinalScoreState::update( float time ) {
  float textY = .5 - FONT_SIZE/2;
  h3dutShowText( scoreText.c_str(), 0.03f, textY, FONT_SIZE * 1.5, 0.9, 0.2, 0.2, gd->font );
  gd->level->spin( time );
}

void FinalScoreState::handleMouseInput( bool down, float x, float y ){ 
  wasDown = isDown;
  isDown = down;
}

GameState *FinalScoreState::checkForChange() {
  if( isDown && ! wasDown ) {
    gd->player->clear();
    gd->level->clear();
    return new MainMenuState( gd );
  }
  return NULL;
}