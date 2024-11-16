#ifndef GENERALDEFS_H
#define GENERALDEFS_H
#include <stdio.h>
#include <time.h>
#include "hit_detect.h"
#include "cpu_update.h"
typedef struct {
    objTree* tree;
    unsigned long objectCount;
    unsigned bufferCount;
} globalInformation;

#endif