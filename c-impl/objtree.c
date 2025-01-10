#include "objtree.h"
#include <stdio.h>
#include <string.h>
#include "settings.h"
objTree* tree_initTree(){
    printf("init pool 1\n");
    tree_allocPool objectpool = tree_allocInitPool(1 * 1e6); //one million (1 MB)
    printf("init pool 2\n");
    tree_allocPool nodepool = tree_allocInitPool(1 * 1e3); //one thousand (1 kB)
    printf("alloc metadata\n");
    objTree* pMetadataStruct = tree_allocate(nodepool, sizeof(objTree));
    printf("alloc searchbuf\n");
    treeNode** searchbuf = tree_allocate(nodepool, SEARCHBUFSIZE*sizeof(treeNode*));
    printf("alloc root node\n");
    treeNode* pRoot = tree_allocate(nodepool, sizeof(treeNode));
    printf("alloc root node buf\n");
    object* rootbuf = tree_allocate(objectpool, sizeof(treeNode)*OBJBUFSIZE);
    *pRoot = (treeNode){
        .buf = rootbuf,
        .places = ~0UL,
        .split.isx = 1,
        .bindrect = (rect_llhh){
            .lowlow = (point){.x = LEFTBORDER, .y = BOTTOMBORDER},
            .highhigh = (point){.x = RIGHTBORDER, .y = TOPBORDER}
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
    printf("delet pool 2\n");   
    tree_allocDestroyPool(tree->objectAllocPool);
    printf("delet pool 1\n");
    tree_allocDestroyPool(tree->nodeAllocPool);

}
treeNode* tree_findParentNode(objTree* tree, object* obj){
    treeNode* currentNode = tree->root;
    while(1) {
        if(currentNode->buf) return currentNode;
        switch(currentNode->split.isx) {
            case 1: //x split
                if(obj->x < currentNode->split.value){
                    printf("descend left\n");
                    currentNode = currentNode->left;
                }
                else {
                    printf("descend right\n");
                    currentNode = currentNode->right;
                }
                break;
            case 0:
                if(obj->y < currentNode->split.value){
                    printf("descend left\n");
                    currentNode = currentNode->left;
                }
                else {
                    printf("descend right\n");
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
    printf("==================\ntree_balanceBuffers:\n");
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
    printf("added up all the coordinates\n");
    double average = accumulator/OBJBUFSIZE; // not the best but cheaper than getting the median 
    unsigned leftcount = 0; //how many objects in given child buffer
    unsigned rightcount = 0;
    parent->split.value = average;
    printf("New split is now set at %c=%lf\n", parent->split.isx ? 'x' : 'y', parent->split.value);
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
    treeNode* other = (void*)0;
    if(parent->buf == left_child->buf){ 
        dest = right_child;
        other = left_child;
        printf("left child buf == parent buf \n");
    }
    else{
        dest = left_child;
        other = right_child;
        printf("right child buf == parent buf \n");
    }  
    printf("==================\ntree_balanceBuffers2:\n");

    
    printf("New split is now set at %c=%lf\n", parent->split.isx ? 'x' : 'y', parent->split.value);
    unsigned destwritten = 0;

    // use precalculated offset to avoid code duplication or checking in the for loop
    unsigned coordoffset = 0;
    if(parent->split.isx) 
        coordoffset = offsetof(object, x);
    else 
        coordoffset = offsetof(object, y);

    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        printf("values compared: %c = %lf and average = %lf\n", 
            parent->split.isx ? 'x' : 'y', 
            *(double*)( (unsigned char*)&(parent->buf[i]) + coordoffset ),
            parent->split.value
        );
        //first value is the x or y field of the i-th struct in buf
        if(*(double*)( (char*)&(parent->buf[i]) + coordoffset )/*same as object->[x/y]*/ > parent->split.value){
            printf("moving object %i to new node\n", i);
            memcpy(dest->buf + destwritten, parent->buf + i, sizeof(object));
            memset(parent->buf + i, '\0', sizeof(object));      //wipe the object (old reused buffer)
            parent->places |= 1UL << i;                         //mark as vacant (old reused buffer)
            printf("doing paperwork\n");
            dest->places &= ~(1UL << destwritten);              //mark as occupied (new buffer)
        }
        else{
            printf("object %d remains where it was\n", i);
        }
    }
    other->places = parent->places;
    parent->places = ~0UL;
    return 0;
}


int tree_insertObject(objTree* tree, object* obj){
    printf("------------------\ntree_insertObject:\n");
    printf("searching for a parent node\n");
    treeNode* parentNode = tree_findParentNode(tree, obj);
    printf("found a parent node\n");
    printf("places: %lx\n", parentNode->places);
    printf("comparison: %x\n", OBJBUFFULLMASK);
    if(!(OBJBUFFULLMASK & parentNode->places)) { //buffer is full
        printf("splitting a node\n");
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
    printf("==================\ntree_splitNode:\n");
    printf("node pointer: %p split coordinate: %c\n", node, node->split.isx ? 'x' : 'y');
    printf("got through ascent phase\n");
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
    printf("added up all the coordinates\n");
    double average = accumulator/OBJBUFSIZE; // not the best but cheaper than getting the median 

    node->split.value = average;
    printf("checked for axis of split. allocating nodes\n");
    node->left = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    node->right = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    fprintf(stderr, "allocated nodes. allocating buffers\n");
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
        }
    };
    *(node->right) = (treeNode){
        .buf = tree_allocate(tree->objectAllocPool, sizeof(object)*OBJBUFSIZE),
        .up = node,
        .places = ~0UL,
        .bindrect = (rect_llhh){
            .lowlow = node->split.isx ? 
                (point){ .x = node->split.value, .y = node->bindrect.lowlow.y}
                :
                (point){ .x = node->bindrect.highhigh.x, .y = node->split.value},
            .highhigh = node->bindrect.highhigh
        }
    };
    tree->bufCount++;
    printf("Parent node has a rectangle of x: %lf -- %lf, y: %lf -- %lf \n", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
    printf("Left child has a rectangle of x: %lf -- %lf, y: %lf -- %lf \n", node->left->bindrect.lowlow.x, node->left->bindrect.highhigh.x, node->left->bindrect.lowlow.y, node->left->bindrect.highhigh.y);
    printf("Right child has a rectangle of x: %lf -- %lf, y: %lf -- %lf \n", node->right->bindrect.lowlow.x, node->right->bindrect.highhigh.x, node->right->bindrect.lowlow.y, node->right->bindrect.highhigh.y);

    fprintf(stderr, "allocated buffers\n");
    tree_balanceBuffers2(node, node->left, node->right);
    node->buf = (void*)0;
    tree->bufCount++;
    return 0;
}