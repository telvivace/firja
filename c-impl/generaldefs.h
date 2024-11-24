#ifndef GENERALDEFS_H
#define GENERALDEFS_H
#include "objtree.h"
typedef struct {
    objTree* tree;
    unsigned long objectCount;
    unsigned bufferCount;
} globalInformation;

#endif