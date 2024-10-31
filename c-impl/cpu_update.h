#ifndef CPU_UPDATE_H
#define CPU_UPDATE_H
#include "objtree.h"
#include "fancy_arrays.h"
#define LEFTBORDER -100
#define RIGHTBORDER 100
#define TOPBORDER 100
#define BOTTOMBORDER -100
int vector_update(object** arr, unsigned size);
int scalar_update(object** arr, unsigned size);
#endif