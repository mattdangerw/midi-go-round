#include "GameManager.h"
#include <Horde3DUtils.h>
#include <math.h>
#include <glm.hpp>
#include <gtx/color_space.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/vector_angle.hpp>
#include <gtx/rotate_vector.hpp>


#define WHEEL_RADIUS 200

inline float randFloat(float low, float high)
{
  return low + (float)rand()/((float)RAND_MAX/(high-low));
}

using namespace glm;

//Takes a note location value from 0 to 1 to an world x coordinate
//stretching from left to right edge of wheel
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

//sets up the wheel and the particle emmiter and such
void Level::init(H3DRes particleSysRes, H3DRes pinwheelRes, H3DRes noteRes,
  H3DRes transMatRes, H3DRes starMatRes ) {
  this->noteRes = noteRes;
  this->starMatRes = starMatRes;

  playerAttach = h3dAddGroupNode( H3DRootNode, "PlayerAttachPoint" );
  h3dSetNodeTransform( playerAttach, 0, WHEEL_RADIUS, 0.25, 0, 0, 0, 1, 1, 1 );

  // Add scene nodes
  turnNode = h3dAddGroupNode( H3DRootNode, "Level" );
  h3dSetNodeTransform( turnNode, 0, 0, 0, 0, 0, 0, 1, 1, 1 );

  player = h3dAddGroupNode( H3DRootNode, "Player" );
  particleSys = h3dAddNodes( player, particleSysRes );

  pinwheel = h3dAddNodes( turnNode, pinwheelRes );
  h3dSetNodeTransform( pinwheel, 0, 0, 0, 0, 0, 0, WHEEL_RADIUS/11.98, WHEEL_RADIUS/11.98, WHEEL_RADIUS/11.98 );

  addTargetLine(transMatRes);

  addStars();
}

//Adds the line on top of the level that lets the user know when a notes about to play.
void Level::addTargetLine(H3DRes transMatRes) {
  float wheelWidth = WHEEL_RADIUS/20.35;
  float posData[] = {
  -wheelWidth, 0, -0.1,
   wheelWidth, 0, -0.1,
  -wheelWidth, 0, 0.1,
   wheelWidth, 0, 0.1
  };
  unsigned int indexData[] = { 0, 1, 2, 2, 1, 3 };
  short normalData[] = {
   0, 1, 0,
   0, 1, 0,
   0, 1, 0,
   0, 1, 0
  };
  float uvData[] = {
   0, 0,
   1, 0,
   0, 1,
   1, 1
  };
  H3DRes geoRes = h3dutCreateGeometryRes( "geoRes", 4, 6, posData, indexData, normalData, 0, 0, uvData, 0 );
  H3DNode rot = h3dAddGroupNode( H3DRootNode, "LineAttach" );
  h3dSetNodeTransform( rot, 0, 0, 0, -0.25, 0, 0, 1, 1, 1 );
  targetLine = h3dAddModelNode( rot, "DynGeoModelNode", geoRes );
  h3dAddMeshNode( targetLine, "DynGeoMesh", transMatRes, 0, 6, 0, 3 );
  h3dSetNodeTransform( targetLine, 0, 199.5, 0, 180, 0, 0, 1, 1, 1 );
  vec4 color(1);
  h3dSetNodeUniforms( targetLine, value_ptr(color), 4 );
}

//Adds the stars in as thousands of billboards! Pretty terrible, but procedural
//textures were being finicky in hoard.
//Also sub pixel sized billboars = cool twinkle effect.
void Level::addStars() {
  //delete
  float posData[] = {
   -1, -1, 0,
   1, -1, 0,
   -1, 1, 0,
   1, 1, 0
  };
  unsigned int indexData[] = { 0, 1, 2, 2, 1, 3 };
  short normalData[] = {
   0, 0, 1,
   0, 0, 1,
   0, 0, 1,
   0, 0, 1
  };
  float uvData[] = {
   0, 0,
   1, 0,
   0, 1,
   1, 1
  };
  
  H3DRes square = h3dutCreateGeometryRes( "square", 4, 6, posData, indexData, normalData, 0, 0, uvData, 0 );
  for(int i = 0; i < 4000; i++){
    H3DNode star = h3dAddModelNode( H3DRootNode, "star" + i, square );
    h3dAddMeshNode( star, "starMesh" + i, starMatRes, 0, 6, 0, 3 );
    vec3 dir = normalize(vec3(randFloat(-1,1), randFloat(-1,1),
      randFloat(-1,0)));
    dir = rotate(dir, 45.0f, vec3(0,1,0));

    vec3 axis = normalize(cross( vec3(0, 0,-1), dir));
    float angle = glm::angle( vec3(0, 0,-1), dir);
    
    // mat4 transform = lookAt(-1.0f*pos, vec3(0), vec3(0,1,0));
    mat4 transform = scale(rotate(translate(mat4(),
      dir*randFloat(3500, 3600)), angle, axis), vec3(randFloat(.02,4)));
    h3dSetNodeTransMat( star, value_ptr(transform) );
  }
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

  vec3 starColor(.5f, .5f, 0.8f);
  float mul = 1.0f / ((time - lastHit) * 2 + .4f);
  starColor += vec3(0.2f, 0.2f, 1.0f) * mul;
  h3dSetMaterialUniform(starMatRes, "myColor", starColor.x, starColor.y, starColor.z, 1.0f);

  // Animate particle systems (several emitters in a group node)
  unsigned int cnt = h3dFindNodes( particleSys, "", H3DNodeTypes::Emitter );
  for( unsigned int i = 0; i < cnt; ++i ){
    h3dAdvanceEmitterTime( h3dGetNodeFindResult( i ), 5 * (time - lastFrameTime) );
  }

  float hueAngle = songProgress * 245 + 115;
  vec4 selectedNoteColor = vec4(rgbColor(vec3(hueAngle, 0.8f, 1.5f)), 1.0f);
  vec4 noteColor = vec4(rgbColor(vec3(hueAngle, .8f, .5f)), 1.0f);
  vec4 wheelColor = vec4(rgbColor(vec3(hueAngle, .8f, .2f)), 0.2f);
  vec4 lineColor = vec4(rgbColor(vec3(hueAngle, .6f, 5.0f)), 1.0f);

  h3dSetNodeUniforms( targetLine, value_ptr(lineColor), 4 );
  h3dSetNodeUniforms( pinwheel, value_ptr(wheelColor), 4 );
  
  // Animate the notes.
  for(int i = 0; i < noteNodes.size(); i++){
    if(!removed[i]){
      int note = notes[i].note;
      float notex = float(note - noteMin) / (noteMax - noteMin);
      if(fabs(notex - playerX) < .1){
        h3dSetNodeUniforms( noteNodes[i], value_ptr(selectedNoteColor), 4 );
      }
      else h3dSetNodeUniforms( noteNodes[i], value_ptr(noteColor), 4 );
      h3dSetNodeTransform( noteNodes[i], 0, .25 * cosf(songProgress * 3.1415926f * 2 / beat) + .25, 
        0, 0, songProgress * 10000, 0, .15, .15, .15 );
    }
  }

  //spin the wheel. 1 rotation during a song
  h3dSetNodeTransform( turnNode, 0, 0, 0, 360 * songProgress, 0, 0, 1, 1, 1 );

  lastFrameTime = time;
}

void Level::unlockPlayer() {
  h3dSetNodeTransform( particleSys, 0, 0, 0, 0, 170, 0, .5, .5, .5 );
  h3dSetNodeTransform( player, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
  h3dSetNodeParent( player, H3DRootNode );
}

void Level::placePlayer( const float *trans ) {
  h3dSetNodeTransMat( player, trans );
}

void Level::lockPlayer() {
  h3dSetNodeTransform( player, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
  h3dSetNodeTransform( particleSys, 0, 0, 0, 0, 180, 0, .5, .5, .5 );
  h3dSetNodeParent( player, playerAttach );
}

void Level::spin( float time ){
  // Animate particle systems (several emitters in a group node)
  unsigned int cnt = h3dFindNodes( particleSys, "", H3DNodeTypes::Emitter );
  for( unsigned int i = 0; i < cnt; ++i ){
    h3dAdvanceEmitterTime( h3dGetNodeFindResult( i ), 5 * (time - lastFrameTime) );
  }
  
  h3dSetNodeTransform( turnNode, 0, 0, 0, time*10, 0, 0, 1, 1, 1 );
  vec4 passiveColor(0.02f, 0.05f, 0.32f, 0.2f);
  h3dSetNodeUniforms( pinwheel, value_ptr(passiveColor), 4 );

  float hueAngle = 115;
  vec4 wheelColor = vec4(rgbColor(vec3(hueAngle, .8f, .2f)), 0.2f);
  vec4 lineColor = vec4(rgbColor(vec3(hueAngle, .6f, 5.0f)), 1.0f);

  h3dSetNodeUniforms( targetLine, value_ptr(lineColor), 4 );
  h3dSetNodeUniforms( pinwheel, value_ptr(wheelColor), 4 );

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
    lastHit = lastFrameTime;
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
  H3DNode attachNode = h3dAddGroupNode( turnNode, "NoteAttachPoint" + noteIndex );
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
  h3dSetNodeTransform( turnNode, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
}

int Level::getScore(){
  return score;
}

//attaches the camera to the top of the level when playing
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