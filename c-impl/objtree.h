#ifndef OBJTREE_H
#define OBJTREE_H
#define OBJBUFSIZE 16
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
struct treeSplit {
    unsigned isx;
    double x;
    double y;
};
typedef struct object object;
struct objTree {
    unsigned depth;
    tree_allocPool* allocPool;
    treeNode* root;
};
typedef struct objTree objTree;
struct treeNode {
    //any of those can be a null pointer
    unsigned leaf; //true or false
    struct treeNode* left;
    struct treeNode* right;
    struct treeSplit split;
    struct treeNode* up;
    object* buf;
    __uint64_t places; //which places in the buffer are vacant?
};
typedef struct treeNode treeNode;
/*
Traverses the BSP tree and gives the address of the node containing the object we need
*/
treeNode* tree_findParentNode(treeNode* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. May split the buffer if required.
*/
int tree_insertObject_s(treeNode* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. Returns an error if the buffer is full.
*/
int tree_insertObject(treeNode* tree, object* obj);
/*
Split a node along a x/y boundary. Might have optimizations laters
*/
int tree_splitNode(treeNode* tree, treeNode* node);


#endif