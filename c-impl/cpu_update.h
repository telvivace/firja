#ifndef CPU_UPDATE_H
#define CPU_UPDATE_H
#include "fancy_arrays.h"
typedef struct {
    float x;
    float y;
} speed;
struct object {
    double x;
    double y;
    speed v;
    float s;
    unsigned m;
    struct object* hit; //optional pointer to an object currently overlapping this one
};
typedef struct object object;
object thing;
int vector_update(obj_arr* arr, unsigned size);
int scalar_update(obj_arr* arr, unsigned size);
#endif