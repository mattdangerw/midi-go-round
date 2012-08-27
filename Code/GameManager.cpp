#include "GameManager.h"
#include <Horde3DUtils.h>
#include <math.h>
#include <iomanip>
#include <iostream>
#include <cstdio>

using namespace std;

// Convert from degrees to radians
inline float degToRad( float f ) 
{
  return f * (3.1415926f / 180.0f);
}

GameManager::GameManager()
{
  debugViewMode = false; wireframeMode = false;
  cam = 0;

  level = new Level();
  player = NULL;
  state = NULL;

  screenshotNextFrame = false;
}

bool GameManager::init( MidiPlayer *p )
{
  player = p;
  //player->init(songFile, getLevel());
  
  // Initialize engine
  if( !h3dInit() ) {	
    h3dutDumpMessages();
    return false;
  }

  // Set options
  h3dSetOption( H3DOptions::LoadTextures, 1 );
  h3dSetOption( H3DOptions::TexCompression, 0 );
  h3dSetOption( H3DOptions::FastAnimation, 1 );
  h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
  h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
  h3dSetOption( H3DOptions::SampleCount, 4 );

  // Add resources
  // Pipelines
  hdrPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0 );
  forwardPipeRes = h3dAddResource( H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
  // Meshes
  H3DRes pinwheelRes = h3dAddResource( H3DResTypes::SceneGraph, "models/pinwheel/test.scene.xml", 0 );
  H3DRes noteRes = h3dAddResource( H3DResTypes::SceneGraph, "models/note/note.scene.xml", 0 );
  // Particle system
  H3DRes particleSysRes = h3dAddResource( H3DResTypes::SceneGraph, "particles/particleSys1/particleSys1.scene.xml", 0 );

  fontRes = h3dAddResource( H3DResTypes::Material, "fonts/font.material.xml", 0 );

  H3DRes transMatRes = h3dAddResource( H3DResTypes::Material, "materials/translucent.material.xml", 0 );

  H3DRes starMatRes = h3dAddResource( H3DResTypes::Material, "materials/star.material.xml", 0 );

  // Load resources
  h3dutLoadResourcesFromDisk( "Content" );

  level->init(particleSysRes, pinwheelRes, noteRes, transMatRes, starMatRes );

  // Add camera
  cam = h3dAddCameraNode( H3DRootNode, "Camera", hdrPipeRes );
  // cam = h3dAddCameraNode( H3DRootNode, "Camera", forwardPipeRes );

  // Customize post processing effects
  H3DNode matRes = h3dFindResource( H3DResTypes::Material, "pipelines/postHDR.material.xml" );
  h3dSetMaterialUniform( matRes, "hdrExposure", 2.0f, 0, 0, 0 );
  h3dSetMaterialUniform( matRes, "hdrBrightThres", 2.0f, 0, 0, 0 );
  h3dSetMaterialUniform( matRes, "hdrBrightOffset", 0.15f, 0, 0, 0 );

  GameData *gd = new GameData();
  gd->level = level;
  gd->player = player;
  gd->font = fontRes;
  gd->cam = cam;
  state = new MainMenuState( gd );

  return true;
}

void GameManager::stop()
{
  delete level;
  delete state->gd;
  delete state;
  // Release engine recourses
  h3dRelease();
}


void GameManager::mainLoop( float time )
{
  //update the game state
  state->update( time );
  GameState *next = state->checkForChange();
  if(next) {
    delete state;
    state = next;
  }

  //Make horde calls to finish frame
  h3dSetOption( H3DOptions::DebugViewMode, debugViewMode ? 1.0f : 0.0f );
  h3dSetOption( H3DOptions::WireframeMode, wireframeMode ? 1.0f : 0.0f );
  // Render scene
  h3dRender( cam );
  // Finish rendering of frame
  h3dFinalizeFrame();
  // Clear the overlays for the next frame
  h3dClearOverlays();
  // Write all messages to log file
  h3dutDumpMessages();

  if(screenshotNextFrame) {
    h3dutScreenshot("screenshot.tga");
    screenshotNextFrame = false;
  }
}

void GameManager::updateMouse( bool down, float mx, float my )
{
  // lastx = mx;
  // lasty = my;
  if( state ) {
    state->handleMouseInput( down, mx, my );
  }
}


void GameManager::resize( int width, int height )
{
  // Resize viewport
  h3dSetNodeParamI( cam, H3DCamera::ViewportXI, 0 );
  h3dSetNodeParamI( cam, H3DCamera::ViewportYI, 0 );
  h3dSetNodeParamI( cam, H3DCamera::ViewportWidthI, width );
  h3dSetNodeParamI( cam, H3DCamera::ViewportHeightI, height );
  
  // Set virtual camera parameters
  h3dSetupCameraView( cam, 45.0f, (float)width / height, 0.1f, 10000.0f );
  h3dResizePipelineBuffers( hdrPipeRes, width, height );
  h3dResizePipelineBuffers( forwardPipeRes, width, height );
}

void GameManager::screenshot() {
  screenshotNextFrame = true;
}