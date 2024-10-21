#ifndef OBJTREE_H
#define OBJTREE_H
#define OBJBUFSIZE 16
#define MAXOBJSIZE 20
#include <ctype.h>
#include <string.h>
#include "tree_alloc.h"
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
struct treeSplit {
    unsigned isx;
    double x;
    double y;
};
struct treeNode {
    //any of those can be a null pointer
    struct treeNode* left;
    struct treeNode* right;
    struct treeSplit split;
    struct treeNode* up;
    object* buf; //if not null, its a leaf!
    __uint64_t places; //which places in the buffer are vacant?
};
typedef struct treeNode treeNode;

struct objTree {
    unsigned depth;
    tree_allocPool* allocPool;
    treeNode* root;
};
typedef struct objTree objTree;

/*
Traverses the BSP tree and gives the address of the node containing the object we need
*/
treeNode* tree_findParentNode(objTree* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. May split the buffer if required.
*/
int tree_insertObject_s(objTree* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. Returns an error if the buffer is full.
*/
int tree_insertObject(objTree* tree, object* obj);
/*
Split a node along a x/y boundary. Might have optimizations laters
*/
int tree_splitNode(objTree* tree, treeNode* node);


#endif