#ifndef SETTINGS_H
#define SETTINGS_H
#include "proc_settings.h"

//Borders of the simulation
 #define LEFTBORDER 0
 #define RIGHTBORDER 1200
 #define TOPBORDER 900
 #define BOTTOMBORDER 0

extern double g_leftborder;
extern double g_rightborder;
extern double g_topborder;
extern double g_bottomborder;
//Gravity (substracted from y vector component)
//note: actually gravity is added to the vector,
//because SDL2 has (0,0) on the top left, so it's
//easier for the user to see it correctly...
 #define GRAVITY 0
//Wall bounciness (0->1 usually except if you're crazy)
//negative values will make things sink into the walls
 #define BOUNCE 1

extern double g_leftborder;
extern double g_rightborder;
extern double g_topborder;
extern double g_bottomborder;
// Screen dimensions
 #define SCREEN_WIDTH 1200
 #define SCREEN_HEIGHT 900

// Logger verbosity (0 -> 1)
 #define ORB_VERBOSE 0
// Graphics mode (0/1)
 #define GRAPHICS_ON 1
// Debug mode (0/1)
 #define DBUG_MODE 0
// Collect timing data and write it out (0/1)
 #define BEING_TIMED 0
// Tree settings
 #define MAXOBJSIZE 20
 #define SEARCHBUFSIZE 40
 #ifndef OBJBUFSIZE
  #define OBJBUFSIZE 8 //max 64 elements, governed by implementation
 #endif
 #ifndef OBJ_DENSITY
  #define OBJ_DENSITY 15000 //how many square units per object
 #endif
 #define OBJBUFFULLMASK ((1UL << OBJBUFSIZE) - 1)
 #define RECYCLE 1
 #define BUILD_TREE_DUMB 0 //builds the tree linearly => very inefficient
#endif