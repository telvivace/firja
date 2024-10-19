#include "objtree.h"
#include "tree_alloc.h"
objTree* tree_findParentNode(objTree* tree, object* obj){
    treeNode* currentNode = tree;
    unsigned larger = 0;
    while(1) {
        if(currentNode->leaf) return currentNode;
        switch(currentNode->split.isx) {
            case 1: //x split
                if(obj->x < currentNode->split.x){
                    currentNode = currentNode->left;
                }
                else {
                    currentNode = currentNode->right;
                }
                break;
            case 2:
                if(obj->y < currentNode->split.y){
                    currentNode = currentNode->left;
                }
                else {
                    currentNode = currentNode->right;
                }
        }
    }
}

int tree_balanceBuffers(object* src, object* dst_left, object* dst_right);

int tree_insertObject(objTree* tree, object* obj){
    treeNode* parentNode = tree_findParentNode(tree, obj);
    if(!parentNode->places) {
        parentNode = tree_splitNode(tree, parentNode); //buffer is full

    }
    else{
        memcpy(parentNode->buf + __builtin_ctzll(parentNode->places), obj, sizeof(object));
        parentNode->places &= ~(1 << __builtin_ctzll(parentNode->places)); //set the bit to 0
    }
    return 0;
}

treeNode* tree_splitNode(objTree* tree, treeNode* node){
    treeNode* currentNode = node;
    unsigned nodeDepth = 0;
    while(currentNode->up){
        nodeDepth++;
        currentNode = currentNode->up;
    }
    if(nodeDepth == tree->depth){
        //big boom bad sad
        return (void*)0;
    }
    else{
        currentNode->left = tree_allocate(tree->allocPool, sizeof(object)*OBJBUFSIZE);
        currentNode->right = tree_allocate(tree->allocPool, sizeof(object)*OBJBUFSIZE);
        tree_balanceBuffers(currentNode->buf, currentNode->left->buf, currentNode->right->buf);
    }
}