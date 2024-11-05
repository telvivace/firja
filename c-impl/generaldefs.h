#ifndef GENERALDEFS_H
#define GENERALDEFS_H
#include <stdio.h>
#include "hit_detect.h"
#include "cpu_update.h"
typedef struct {
    objTree* tree;
    unsigned long objectCount;
    unsigned bufferCount;
} globalInfo;

#endif