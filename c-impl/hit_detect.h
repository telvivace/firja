#ifndef HITDETECT_H
#define HITDETECT_H
#include "objtree.h"
int hit_flagObjects(objTree* tree);
int hit_findRect_shallow(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static SEARCHBUFSIZE]);
int tree_findRect_deep(objTree* tree, treeNode* node, rect_ofex query, object* results[static SEARCHBUFSIZE]);
#endif