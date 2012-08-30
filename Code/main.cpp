// *************************************************************************************************
//
// Midi-Go-Round
// Matt Watson
// mattdangerw@gmail.com
//
// This file has all the nasty setting up code.
// Gets RTAudio up and running. Sets up GLFW and passes user input down to the game manager.
// Sets up Horde and starts the main game loop.
//
// *************************************************************************************************

#include "MidiPlayer.h"
#include "GameManager.h"
#include "RtAudio.h"
#include <cstdlib>
#include <iostream>
#include <glfw.h>

#define SAMPLE float
#define MY_SRATE 44100

using namespace std;

MidiPlayer *player = NULL;
GameManager *manager = NULL;
RtAudio *audio = NULL;

bool running = false;
bool mouseDown = false;
int width = 800, height = 600;

void quit();

void cleanup();

int windowCloseListener();

void keyPressListener(int key, int action);

void mouseMoveListener(int x, int y);

void mouseButtonListener(int button, int state);

int audioCallback( void * outputBuffer, void * inputBuffer, 
       unsigned int bufferSize, double streamTime,
       RtAudioStreamStatus status, void * userData );

int main(int argc, const char* argv[]) {    
  // pointer to RtAudio object
  unsigned int bufferSize = 128;

  // create the object
  try {
    audio = new RtAudio();
    cerr << "buffer size: " << bufferSize << endl;
  }
  catch( RtError & err ) {
    err.printMessage();
    exit(1);
  }

  if( audio->getDeviceCount() < 1 ) {
    // nopes
    cout << "no audio devices found!" << endl;
    exit( 1 );
  }


  // let RtAudio print messages to stderr.
  audio->showWarnings( true );

  // set input and output parameters
  RtAudio::StreamParameters iParams, oParams;
  iParams.deviceId = audio->getDefaultInputDevice();
  iParams.nChannels = 1;
  iParams.firstChannel = 0;
  oParams.deviceId = audio->getDefaultOutputDevice();
  oParams.nChannels = 2;
  oParams.firstChannel = 0;
        
  // create stream options
  RtAudio::StreamOptions options;

  player = new MidiPlayer();
  manager = new GameManager();

  // set the callback and start stream
  try {
    audio->openStream( &oParams, &iParams, RTAUDIO_FLOAT32, MY_SRATE, &bufferSize, &audioCallback, NULL, &options);
    audio->startStream();
      
    // test RtAudio functionality for reporting latency.
    cout << "stream latency: " << audio->getStreamLatency() << " frames" << endl;
  }
  catch( RtError & err ) {
    err.printMessage();
    cleanup();
  }

  glfwInit();
  glfwEnable( GLFW_STICKY_KEYS );
  GLFWvidmode mode;
  glfwGetDesktopMode( &mode );
  width = mode.Width;
  height = mode.Height;
  if(!glfwOpenWindow( width, height, 8, 8, 8, 8, 24, 8, GLFW_FULLSCREEN) ){
    cout << "Could not open window" << endl;
    cleanup();
  }

  // Disable vertical synchronization
  // glfwSwapInterval( 0 );

  // Set listeners
  glfwSetWindowCloseCallback( windowCloseListener );
  glfwSetKeyCallback( keyPressListener );
  glfwSetMousePosCallback( mouseMoveListener );
  glfwSetMouseButtonCallback( mouseButtonListener );

  // Initialize game and engine
  glfwSetWindowTitle( "Midi-Go-Round" );

  if ( !manager->init(player) ){
    cout << "Could not open window" << endl;
    cleanup();
  }  
  manager->resize( width, height );

  //Game loop
  running = true;
  int frame = 0;
  float lastTime = glfwGetTime();
  while( running ){
    float currTime = glfwGetTime();
    frame++;
    if( frame % 1000 == 0 ){
      cout << "FPS: " << 1000 / (currTime - lastTime) << endl;
      lastTime = currTime;
    }
    
    manager->mainLoop( currTime );
    glfwSwapBuffers();
  }
  
  // if we get here, stop!
  try {
    audio->stopStream();
  }
  catch( RtError & err ) {
    err.printMessage();
  }

  cleanup();
  return 0;
}

//We won't use this here in main. But can be forward declared.
void quit(){
  running = false;
}

void cleanup(){
  glfwTerminate();

  if(manager) {
    manager->stop();
    delete manager;
  }

  if(audio) {
    audio->closeStream();
    delete audio;
  }

  exit(0);
}

int windowCloseListener()
{
  running = false;
  return 0;
}

void keyPressListener( int key, int action )
{
  if( action == GLFW_PRESS ) {
    if(key == GLFW_KEY_ESC) {
      running = false;
    }
    if(key == 'S') {
      manager->screenshot();
    }
  }
}

float clamp( float c )
{
  if( c < 0 ) return 0;
  if( c > 1 ) return 1;
  return c;
}

void mouseMoveListener( int x, int y )
{
  manager->updateMouse( mouseDown, clamp((float)(x)/width), clamp((float)(y)/height) );
}

void mouseButtonListener( int button, int state ){
  if(button == GLFW_MOUSE_BUTTON_LEFT){
    if(state == GLFW_PRESS){
      mouseDown = true;
    }
    else{
      mouseDown = false;
    }
    int x, y;
    glfwGetMousePos(&x, &y);
    manager->updateMouse( mouseDown, clamp((float)(x)/width), clamp((float)(y)/height) );
  }
}

int audioCallback( void * outputBuffer, void * inputBuffer, 
       unsigned int bufferSize, double streamTime,
       RtAudioStreamStatus status, void * userData )
{
  SAMPLE *out = (SAMPLE *)outputBuffer;
        
  player->generate(out, bufferSize);

  return 0;
}
