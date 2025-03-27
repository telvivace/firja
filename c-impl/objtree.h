#ifndef OBJTREE_H
#define OBJTREE_H
#include <stdint.h>
#include "tree_alloc.h"
#include "settings.h"
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
    unsigned long id;
    unsigned flags;
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
    uint64_t places; //bitmask: vacant place = 1, filled = 0, start at least significant
    rect_llhh bindrect;
    unsigned level; //how deep is the node within the tree
    uint16_t flags;
    uint16_t optindex;
};
typedef struct treeNode treeNode;
typedef struct {
    unsigned nodecount;
    unsigned allocated;
    treeNode** nodes;
} treeOptimizationQueue;

typedef struct {
    unsigned start;
    unsigned length;
} hitGroup;

typedef struct {
    unsigned groupcount;
    unsigned allocated;
    hitGroup* groups;
} hitGroupQueue;

typedef struct {
    unsigned objcount;
    unsigned allocated;
    object* objects;
} hitObjectQueue;

struct objTree {
    unsigned depth;
    unsigned searchbufsize;
    tree_allocPool objectAllocPool;
    tree_allocPool nodeAllocPool;
    treeNode* root;
    object** searchbuf;
    hitObjectQueue hitObjectQueue;
    hitGroupQueue hitGroupQueue;
    unsigned bufCount;
    unsigned long validObjCount;
    unsigned long relocations;
    treeOptimizationQueue opt_queue;
};
typedef struct objTree objTree;
/*
Initializes the tree.
*/
objTree* tree_initTree(void);
/*
Destroys the tree.
*/
void tree_deleteTree(objTree* tree);
/*
Traverses the tree and gives the address of the node containing the object we need
*/
treeNode* tree_findParentNode(objTree* tree, object* obj);
/*
Finds a vacant place in the object buffer of a leaf node. Returns an error if the buffer is full.
*/
int tree_insertObject(objTree* tree, object* obj);
/*
Split a node into two along a boundary parallel to the x/y axes.
*/
int tree_splitNode(objTree* tree, treeNode* node);
/*
Submit a node into the optimization queue
*/
int tree_OptqueueSubmit(objTree* tree, treeNode* node);
/*
Optimize the tree using the optimization queue
*/
int tree_optimizeNodes(objTree* tree);
#endif