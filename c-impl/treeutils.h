#ifndef TREEUTILS_H
#define TREEUTILS_H
#include "objtree.h"
#include <stdio.h>
#include "liblogs.h"
int tree_printTree(objTree* tree);

int tree_printTreeBoxes(objTree* tree);

unsigned countSetBitsUL(unsigned long N);

static inline void tree_printObject(object* obj){
    orb_logf(PRIORITY_TRACE,"id: %ld\n x: %lf  y: %lf\n s: %lf\n",
    obj->id, obj->x, obj->y, obj->s);
}
#endif