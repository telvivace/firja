#ifndef HITDETECT_H
#define HITDETECT_H
#include "objtree.h"
#include <tgmath.h>
int hit_flagObjects(objTree* tree);
int hit_findRect_shallow(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static 12]);
int tree_findRect_deep(objTree* tree, treeNode* node, rect_ofex query, object* results[static 12]);
#endif