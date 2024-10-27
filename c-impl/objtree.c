#include "objtree.h"
treeNode* tree_findParentNode(objTree* tree, object* obj){
    treeNode* currentNode = tree->root;
    while(1) {
        if(currentNode->buf) return currentNode;
        switch(currentNode->split.isx) {
            case 1: //x split
                if(obj->x < currentNode->split.value){
                    currentNode = currentNode->left;
                }
                else {
                    currentNode = currentNode->right;
                }
                break;
            case 2:
                if(obj->y < currentNode->split.value){
                    currentNode = currentNode->left;
                }
                else {
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
    
    double average = accumulator/OBJBUFSIZE; // not the best but cheaper than getting the median 
    unsigned leftcount = 0; //how many objects in given child buffer
    unsigned rightcount = 0;
    parent->split.value = average;
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

int tree_insertObject(objTree* tree, object* obj){
    treeNode* parentNode = tree_findParentNode(tree, obj);
    if(!parentNode->places) {
        if(tree_splitNode(tree, parentNode) != 0) return 1; //buffer is full
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
    parentNode->places &= ~(1 << __builtin_ctzl(parentNode->places)); //set the bit to 0
    return 0;
}

int tree_splitNode(objTree* tree, treeNode* node){
    treeNode* currentNode = node;
    unsigned nodeDepth = 0;
    while(currentNode->up){
        nodeDepth++;
        currentNode = currentNode->up;
    }
    if(nodeDepth == tree->depth){
        tree->depth++;
    }
    currentNode->left = tree_allocate(tree->allocPool, sizeof(object)*OBJBUFSIZE);
    currentNode->right = tree_allocate(tree->allocPool, sizeof(object)*OBJBUFSIZE);
    tree_balanceBuffers(currentNode, currentNode->left, currentNode->right);
    tree_free(currentNode->buf);
    return 0;
}