#ifndef SETTINGS_H
#define SETTINGS_H
//Borders of the simulation
 #define LEFTBORDER 0
 #define RIGHTBORDER 1200
 #define TOPBORDER 900
 #define BOTTOMBORDER 0
//Gravity (substracted from y vector component)
//note: actually gravity is added to the vector,
//because SDL2 has (0,0) on the top left, so it's
//easier for the user to see it correctly...
 #define GRAVITY 0
//Wall bounciness (0->1 usually except if you're crazy)
//negative values will make things sink into the walls
 #define BOUNCE 1

// Screen dimensions
 #define SCREEN_WIDTH 1200
 #define SCREEN_HEIGHT 900
// Logger verbosity (0 -> 1)
 #define ORB_VERBOSE 0
// Graphics mode (0/1)
 #define GRAPHICS_ON 1
// Debug mode (0/1)
 #define DBUG_MODE 0

//tree settings
 #define MAXOBJSIZE 20
 #define SEARCHBUFSIZE 40
 #define OBJBUFSIZE 8 //max 64 elements, governed by implementation
 #define OBJBUFFULLMASK ((1UL << OBJBUFSIZE) - 1)

#endif