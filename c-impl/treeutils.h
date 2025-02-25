#ifndef TREEUTILS_H
#define TREEUTILS_H
#include "objtree.h"
#include <stdio.h>
#include "liblogs.h"
int tree_printTree(objTree* tree);

int tree_printTreeBoxes(objTree* tree);

unsigned countSetBitsUL(unsigned long N);

static inline void tree_printObject(enum log_priorities priority,  object* obj){
    orb_logf(priority,"id: %ld\n x: %lf  y: %lf\n s: %lf v: (%lf, %lf) flags: %x",
    obj->id, obj->x, obj->y, obj->s, obj->v.x, obj->v.y, obj->flags);
}
void tree_printBufferContents(enum log_priorities priority, object* buf);
int tree_writeUintBufferToFile(const unsigned* buf, size_t size, char* foldername, char* filename);
#endif