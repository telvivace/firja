#ifndef OBJTREE_H
#define OBJTREE_H
#define OBJBUFSIZE 16 //max 64 elements, governed by implementation
#define OBJBUFFULLMASK 65535 //2^(OBJBUFSIZE)-1
#define MAXOBJSIZE 20
#define SEARCHBUFSIZE 40
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include "tree_alloc.h"
typedef struct {
    float x;
    float y;
} speed;
typedef struct {
    double x;
    double y;
} point;
typedef struct {
    double width;
    double height;
} extent;
typedef struct {
    point offset;
    extent extent;
} rect_ofex;
typedef struct {
    point lowlow;
    point highhigh;
} rect_llhh;
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
    double value;
};
typedef struct treeSplit treeSplit;
struct treeNode {
    //any of those can be a null pointer
    struct treeNode* left;
    struct treeNode* right;
    struct treeSplit split;
    struct treeNode* up;
    object* buf; //if not null, its a leaf!
    __uint64_t places; //bitmask: vacant place = 1, filled = 0, start at right
};
typedef struct treeNode treeNode;

struct objTree {
    unsigned depth;
    unsigned searchbufsize;
    tree_allocPool* objectAllocPool;
    tree_allocPool* nodeAllocPool;
    treeNode* root;
    treeNode** searchbuf;
    unsigned bufCount;
};
typedef struct objTree objTree;
/*
Initializes the k/d tree.
*/
objTree* tree_initTree(void);
/*
Destroys the k/d tree.
*/
void tree_deleteTree(objTree* tree);
/*
Traverses the BSP tree and gives the address of the node containing the object we need
*/
treeNode* tree_findParentNode(objTree* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. Returns an error if the buffer is full.
*/
int tree_insertObject(objTree* tree, object* obj);
/*
Split a node along a x/y boundary. Might have optimizations laters
*/
int tree_splitNode(objTree* tree, treeNode* node);


#endif