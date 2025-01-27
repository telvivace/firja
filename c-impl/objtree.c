#include <stdio.h>
#include <string.h>
#include "objtree.h"
#include "treeutils.h"
#include "settings.h"
#include "liblogs.h"
objTree* tree_initTree(){
    orb_logf(PRIORITY_DBUG,"init pool 1");
    tree_allocPool objectpool = tree_allocInitPool(1 * 1e6); //one million (1 MB)
    orb_logf(PRIORITY_DBUG,"init pool 2");
    tree_allocPool nodepool = tree_allocInitPool(1 * 1e3); //one thousand (1 kB)
    orb_logf(PRIORITY_DBUG,"alloc metadata");
    objTree* pMetadataStruct = tree_allocate(nodepool, sizeof(objTree));
    orb_logf(PRIORITY_DBUG,"alloc searchbuf");
    treeNode** searchbuf = tree_allocate(nodepool, SEARCHBUFSIZE*sizeof(treeNode*));
    orb_logf(PRIORITY_DBUG,"alloc root node");
    treeNode* pRoot = tree_allocate(nodepool, sizeof(treeNode));
    orb_logf(PRIORITY_DBUG,"alloc root node buf");
    object* rootbuf = tree_allocate(objectpool, sizeof(treeNode)*OBJBUFSIZE);
    *pRoot = (treeNode){
        .buf = rootbuf,
        .places = ~0UL,
        .split.isx = 1,
        .bindrect = (rect_llhh){
            .lowlow = (point){.x = LEFTBORDER - 20, .y = BOTTOMBORDER - 20},
            .highhigh = (point){.x = RIGHTBORDER + 20, .y = TOPBORDER + 20}
        }
    };
    *pMetadataStruct = (objTree){
        .searchbuf = searchbuf,
        .searchbufsize = SEARCHBUFSIZE,
        .root = pRoot,
        .bufCount = 1, //root node
    };
    pMetadataStruct->objectAllocPool = objectpool;
    pMetadataStruct->nodeAllocPool = nodepool;
    return pMetadataStruct;
}
void tree_deleteTree(objTree* tree){
    orb_logf(PRIORITY_DBUG,"delet pool 2");   
    tree_allocDestroyPool(tree->objectAllocPool);
    orb_logf(PRIORITY_DBUG,"delet pool 1");
    tree_allocDestroyPool(tree->nodeAllocPool);

}
treeNode* tree_findParentNode(objTree* tree, object* obj){
    treeNode* currentNode = tree->root;
    while(1) {
        if(currentNode->buf) return currentNode;
        switch(currentNode->split.isx) {
            case 1: //x split
                if(obj->x < currentNode->split.value){
                    orb_logf(PRIORITY_DBUG,"descend left");
                    currentNode = currentNode->left;
                }
                else {
                    orb_logf(PRIORITY_DBUG,"descend right");
                    currentNode = currentNode->right;
                }
                break;
            case 0:
                if(obj->y < currentNode->split.value){
                    orb_logf(PRIORITY_DBUG,"descend left");
                    currentNode = currentNode->left;
                }
                else {
                    orb_logf(PRIORITY_DBUG,"descend right");
                    currentNode = currentNode->right;
                }
        }
    }
}
int tree_balanceBuffers(treeNode* parent, treeNode* left_child, treeNode* right_child){
    // ========================
    //  WARN FIXME leads to inaccuracy if the coordinates are large! This system uses absolute coordinates!
    //  The issue may be avoided by using multiple relative coordinate systems later on
    //  Also its not that bad, it starts getting actually bad at around 10^7 or more, so we're  
    //  kinda okay if we keep the numbers low
    // ========================
    orb_logf(PRIORITY_DBUG,"==================\ntree_balanceBuffers:");
    double accumulator = 0;
    if(parent->split.isx){
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            accumulator += parent->buf[i].x; 
        };
    }
    else{
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            accumulator += parent->buf[i].y; 
        };
    }
    orb_logf(PRIORITY_DBUG,"added up all the coordinates");
    double average = accumulator/OBJBUFSIZE; // not the best but cheaper than getting the median 
    unsigned leftcount = 0; //how many objects in given child buffer
    unsigned rightcount = 0;
    parent->split.value = average;
    orb_logf(PRIORITY_TRACE,"New split is now set at %c=%lf", parent->split.isx ? 'x' : 'y', parent->split.value);
    left_child->places = ~0UL;
    right_child->places = ~0UL;
    if(parent->split.isx){ //split based on the average
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            if(parent->buf[i].x < average){
                memcpy(left_child->buf + leftcount, parent->buf + i, sizeof(object));
                left_child->places &= ~(1UL << leftcount);
                leftcount++;
            }
            else{
                memcpy(right_child->buf + rightcount, parent->buf + i, sizeof(object));
                right_child->places &= ~(1UL << rightcount);
                rightcount++;
            }
        }
    }
    else{//same but for y... didnt find a good way to reuse code
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            if(parent->buf[i].y < average){
                memcpy(left_child->buf + leftcount, parent->buf + i, sizeof(object));
                left_child->places &= ~(1UL << leftcount);
                leftcount++;

            }
            else{
                memcpy(right_child->buf + rightcount, parent->buf + i, sizeof(object));
                right_child->places &= ~(1UL << rightcount);
                rightcount++;
            }
        }
    }
    return 0;
    
}
/*
this function expects that the buffers of the parent and exactly one child are identical in order to save space.
*/
int tree_balanceBuffers2(treeNode* parent, treeNode* left_child, treeNode* right_child){
    treeNode* dest = (void*)0;
    treeNode* reused = (void*)0;
    if(parent->buf == left_child->buf){ 
        dest = right_child;
        reused = left_child;
        orb_logf(PRIORITY_TRACE,"left child buf == parent buf ");
    }
    else{
        dest = left_child;
        reused = right_child;
        orb_logf(PRIORITY_TRACE,"right child buf == parent buf ");
    }  
    orb_logf(PRIORITY_DBUG,"==================\ntree_balanceBuffers2:");
    orb_logf(PRIORITY_TRACE,"parent's places: %lx", parent->places);
    
    orb_logf(PRIORITY_TRACE,"New split is now set at %c=%lf", parent->split.isx ? 'x' : 'y', parent->split.value);
    unsigned destwritten = 0;

    // use precalculated offset to avoid code duplication or checking in the for loop
    unsigned coordoffset = 0;
    if(parent->split.isx) 
        coordoffset = offsetof(object, x);
    else 
        coordoffset = offsetof(object, y);

    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        tree_printObject(parent->buf + i);
        orb_logf(PRIORITY_TRACE,"values compared: %c = %lf and average = %lf", 
            parent->split.isx ? 'x' : 'y', 
            *(double*)( (unsigned char*)&(parent->buf[i]) + coordoffset ),
            parent->split.value
        );
        //first value is the x or y field of the i-th struct in buf
        if(*(double*)( (char*)&(parent->buf[i]) + coordoffset )/*same as object->[x/y]*/ > parent->split.value){
            orb_logf(PRIORITY_TRACE,"moving object %i to new node", i);
            orb_logf(PRIORITY_TRACE,"writing to dest + %d", destwritten);
            memcpy(dest->buf + destwritten, parent->buf + i, sizeof(object));
            memset(parent->buf + i, '\0', sizeof(object));      //wipe the object (old reused buffer)
            parent->places |= 1UL << i;                         //mark as vacant (old reused buffer)
            orb_logf(PRIORITY_DBUG,"doing paperwork");
            dest->places &= ~(1UL << destwritten);              //mark as occupied (new buffer)
            destwritten++;
        }
        else{
            orb_logf(PRIORITY_TRACE,"object %d remains where it was", i);
        }
    }
    reused->places = parent->places;
    //orb_logf(PRIORITY_TRACE,"reused's places: %lx - count: %d\n new's places: %lx - count: %d")
    parent->places = ~0UL;
    return 0;
}


int tree_insertObject(objTree* tree, object* obj){
    orb_logf(PRIORITY_DBUG,"------------------\ntree_insertObject:");
    orb_logf(PRIORITY_DBUG,"searching for a parent node");
    treeNode* parentNode = tree_findParentNode(tree, obj);
    orb_logf(PRIORITY_DBUG,"found a parent node");
    orb_logf(PRIORITY_TRACE,"places: %lx", parentNode->places);
    orb_logf(PRIORITY_TRACE,"comparison: %x", OBJBUFFULLMASK);
    if(!(OBJBUFFULLMASK & parentNode->places)) { //buffer is full
        orb_logf(PRIORITY_DBUG,"splitting a node");
        if(tree_splitNode(tree, parentNode) != 0) return 1; 
        if(parentNode->split.isx){
            if(obj->x < parentNode->split.value){
                parentNode = parentNode->left;
            }
            else {
                parentNode = parentNode->right;
            }
        } 
        else {
            if(obj->y < parentNode->split.value){
                parentNode = parentNode->left;
            }
            else {
                parentNode = parentNode->right;
            }
        }
    }
    memcpy(parentNode->buf + __builtin_ctzl(parentNode->places), obj, sizeof(object));
    parentNode->places &= ~(1 << __builtin_ctzl(parentNode->places)); //mark as occupied
    return 0;
}

int tree_splitNode(objTree* tree, treeNode* node){
    orb_logf(PRIORITY_DBUG,"==================\ntree_splitNode:");
    orb_logf(PRIORITY_TRACE,"node pointer: %p split coordinate: %c", node, node->split.isx ? 'x' : 'y');
    orb_logf(PRIORITY_DBUG,"got through ascent phase");
    if(node->up){
        // if parent is y, then this is x.
        // the opposite is guaranteed by calloc
        if(!node->up->split.isx) node->split.isx = 1;
    }
    //the root is always x
    else node->split.isx = 1;
    double accumulator = 0;
    if(node->split.isx){
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            accumulator += node->buf[i].x; 
        };
    }
    else{
        for(unsigned i = 0; i < OBJBUFSIZE; i++){
            accumulator += node->buf[i].y; 
        };
    }
    orb_logf(PRIORITY_DBUG,"added up all the coordinates");
    double average = accumulator/OBJBUFSIZE; // not the best but cheaper than getting the median 

    node->split.value = average;
    orb_logf(PRIORITY_DBUG,"checked for axis of split. allocating nodes");
    node->left = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    node->right = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    orb_logf(PRIORITY_DBUG, "allocated nodes. allocating buffers");
    *(node->left) = (treeNode){
        .buf = node->buf, //reuse parent buf. Only works with balanceBuffers2
        .up = node,
        .places = ~0UL,
        .bindrect = (rect_llhh){
            .lowlow = node->bindrect.lowlow,
            .highhigh = node->split.isx ? 
                (point){ .x = node->split.value, .y = node->bindrect.highhigh.y}
                :
                (point){ .x = node->bindrect.highhigh.x, .y = node->split.value}
        },
        .level = node->level + 1,
    };
    *(node->right) = (treeNode){
        .buf = tree_allocate(tree->objectAllocPool, sizeof(object)*OBJBUFSIZE),
        .up = node,
        .places = ~0UL,
        .bindrect = (rect_llhh){
            .lowlow = node->split.isx ? 
                (point){ .x = node->split.value, .y = node->bindrect.lowlow.y}
                :
                (point){ .x = node->bindrect.lowlow.x, .y = node->split.value},
            .highhigh = node->bindrect.highhigh
        },
        .level = node->level + 1,
    };
    tree->bufCount++;
    orb_logf(PRIORITY_TRACE,"Parent node has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
    orb_logf(PRIORITY_TRACE,"Left child has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->left->bindrect.lowlow.x, node->left->bindrect.highhigh.x, node->left->bindrect.lowlow.y, node->left->bindrect.highhigh.y);
    orb_logf(PRIORITY_TRACE,"Right child has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->right->bindrect.lowlow.x, node->right->bindrect.highhigh.x, node->right->bindrect.lowlow.y, node->right->bindrect.highhigh.y);

    orb_logf(PRIORITY_DBUG, "allocated buffers");
    tree_balanceBuffers2(node, node->left, node->right);
    node->buf = (void*)0;
    return 0;
}