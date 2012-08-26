#include "GameManager.h"
#include <Horde3DUtils.h>
#include <math.h>

#define WHEEL_RADIUS 200

//Takes a note location value from 0 to 1 to an world x coordinate stretching from left to right edge of wheel
inline float toWheelX( float c ) 
{
  return (c -.5f) * (WHEEL_RADIUS/12.);
}

Level::Level(){
  lastFrameTime = -1;

  playerX = .5;

  noteMin = 256;
  noteMax = -1;

  score = 0;
  streak = 0;

  beat = 0;
}

Level::~Level(){
}

void Level::init(H3DRes particleSysRes, H3DRes pinwheelRes, H3DRes noteRes) {
  this->noteRes = noteRes;

  playerAttach = h3dAddGroupNode( H3DRootNode, "PlayerAttachPoint" );
  h3dSetNodeTransform( playerAttach, 0, WHEEL_RADIUS, 0.25, 0, 0, 0, 1, 1, 1 );

  // Add scene nodes
  levelWheel = h3dAddGroupNode( H3DRootNode, "Level" );
  h3dSetNodeTransform( levelWheel, 0, 0, 0, 0, 0, 0, 1, 1, 1 );

  player = h3dAddGroupNode( H3DRootNode, "Player" );
  particleSys = h3dAddNodes( player, particleSysRes );
  h3dSetNodeTransform( particleSys, 0, 0, 0, 0, 180, 0, .5, .5, .5 );


  H3DNode pinwheel = h3dAddNodes( levelWheel, pinwheelRes );
  h3dSetNodeTransform( pinwheel, 0, 0, 0, 0, 0, 0, WHEEL_RADIUS/11.98, WHEEL_RADIUS/11.98, WHEEL_RADIUS/11.98 );

  // // Add light source
  // H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
  // h3dSetNodeTransform( light, 0, 0, 0, -90, 0, 0, 1, 1, 1 );
  // h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 30 );
  // h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
  // h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 1 );
  // h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.01f );
  // h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 1.0f );
  // h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.8f );
  // h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.7f );
  // h3dSetNodeParamF( light, H3DLight::ColorMultiplierF, 0, 1.0f );
}

void Level::clear(){
  lastFrameTime = -1;

  playerX = .5;

  noteMin = 256;
  noteMax = -1;

  score = 0;
  streak = 0;

  beat = 0;

  for (int i = 0; i < noteNodes.size(); i++)
  {
    if(!removed[i]){
      h3dRemoveNode(noteNodes[i]);
      h3dRemoveNode(attachPoints[i]);
    }
  }
   
  removed.clear();
  toRemove.clear();
  notes.clear();
  groupMap.clear();
  noteNodes.clear();
  attachPoints.clear();
}

void Level::update( float time, double songProgress ){
  removeHits();

  // Animate particle systems (several emitters in a group node)
  unsigned int cnt = h3dFindNodes( particleSys, "", H3DNodeTypes::Emitter );
  for( unsigned int i = 0; i < cnt; ++i ){
    h3dAdvanceEmitterTime( h3dGetNodeFindResult( i ), 5 * (time - lastFrameTime) );
  }

  // Animate the notes.
  for(int i = 0; i < noteNodes.size(); i++){
    if(!removed[i]){
      h3dSetNodeTransform( noteNodes[i], 0, .25 * cosf(songProgress * 3.1415926f * 2 / beat) + .25, 
        0, 0, songProgress * 10000, 0, .15, .15, .15 );
    }
  }

  //spin the wheel. 1 rotation during a song
  h3dSetNodeTransform( levelWheel, 0, 0, 0, 360 * songProgress, 0, 0, 1, 1, 1 );

  lastFrameTime = time;
}

void Level::unlockPlayer() {
  h3dSetNodeTransform( player, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
  h3dSetNodeParent( player, H3DRootNode );
}

void Level::placePlayer( float x, float y, float z ) {
  h3dSetNodeTransform( player, x, y, z, 0, 270, 0, 1, 1, 1 );
}

void Level::lockPlayer() {
  h3dSetNodeTransform( player, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
  h3dSetNodeParent( player, playerAttach );
}

void Level::spin( float time ){
  // Animate particle systems (several emitters in a group node)
  unsigned int cnt = h3dFindNodes( particleSys, "", H3DNodeTypes::Emitter );
  for( unsigned int i = 0; i < cnt; ++i ){
    h3dAdvanceEmitterTime( h3dGetNodeFindResult( i ), 5 * (time - lastFrameTime) );
  }
  
  h3dSetNodeTransform( levelWheel, 0, 0, 0, time*10, 0, 0, 1, 1, 1 );

  lastFrameTime = time;
}

void Level::removeHits(){
  for(int i = 0; i < toRemove.size(); i++) {
    int noteIndex = toRemove[i];
    h3dRemoveNode(noteNodes[noteIndex]);
    h3dRemoveNode(attachPoints[noteIndex]);
    removed[noteIndex] = true;
  }
  toRemove.clear();
}

void Level::handleMouseInput(float mx)
{
  playerX = mx;
  h3dSetNodeTransform( player, toWheelX(mx), 0, 0, 0, 0, 0, 1, 1, 1 );
}

//called in audio callback not main game thread. 
bool Level::checkNote(int group){
  vector<int> *vec = &groupMap[group];
  bool hit = false;
  for(int i = 0; i < vec->size(); i++){
     int noteIndex = vec->at(i);
     int note = notes[noteIndex].note;
     float notex = float(note - noteMin) / (noteMax - noteMin);
     if(fabs(notex - playerX) < .1){
      hit = true;
      break;
    }
  }
  if(hit) {
    streak++;
    if(streak > 50){
      score += 500;
    }
    else if(streak > 10){
      score += 100;
    }
    else score += 50;

    //removing scene nodes here is not thread safe. add to list toRemove
    //push_back is not thread safe either. add in lock?
    for(int i = 0; i < vec->size(); i++){
      toRemove.push_back(vec->at(i));
    }
  }
  else streak = 0;
  return hit;
}

void Level::setBeat( double b ){
  beat = b;
}


void Level::setNote(int group, int note, double onset){
  if(note < noteMin) noteMin = note;
  if(note > noteMax) noteMax = note;

  int noteIndex = notes.size();
  Note n;
  n.group = group;
  n.note = note;
  n.onset = onset;
  notes.push_back(n);
  
  if(groupMap.count(group) == 0){
    groupMap[group] = vector<int>();
  }
  groupMap[group].push_back(noteIndex);
}

void Level::addNoteObj(int noteIndex, float degrees, float c){
  degrees = 360 - degrees;
  H3DNode attachNode = h3dAddGroupNode( levelWheel, "NoteAttachPoint" + noteIndex );
  float rads = degrees * (3.1415926f / 180.0f);
  h3dSetNodeTransform( attachNode, toWheelX(c), WHEEL_RADIUS * cosf(rads), WHEEL_RADIUS * sinf(rads), degrees, 0, 0, 1, 1, 1 );
  H3DNode noteNode = h3dAddNodes( attachNode, noteRes );
  h3dSetNodeTransform( noteNode, 0, 0, 0, 0, 0, 0, .15, .15, .15 );
  attachPoints.push_back(attachNode);
  noteNodes.push_back(noteNode);
  removed.push_back(false);
}

void Level::finalizeNotes(){
  for(int i = 0; i < notes.size(); i++){
    Note n = notes[i];
    float angle = 360 * n.onset;
    float c = float(n.note - noteMin) / (noteMax - noteMin);
    addNoteObj(i, angle, c);
  }
  h3dSetNodeTransform( levelWheel, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
}

int Level::getScore(){
  return score;
}

void Level::attachCamera(H3DNode cam){
  h3dSetNodeParent(cam, player);
  h3dSetNodeTransform( cam, 0, 12, 25, -15, 0, 0, 1, 1, 1 );
}

int Level::getPowerup(){
  if(streak > 50){
    return 2;
  }
  else if(streak > 10){
    return 1;
  }
  return 0;
}