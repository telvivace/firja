#include "objtree.h"
objTree* tree_initTree(){
    tree_allocPool* objectpool = tree_allocInitPool(1 * 1e6); //one million (1 MB)
    tree_allocPool* nodepool = tree_allocInitPool(1 * 1e3); //one million (1 kB)
    objTree* pMetadataStruct = tree_allocate(nodepool, sizeof(objTree));
    treeNode** searchbuf = tree_allocate(nodepool, SEARCHBUFSIZE*sizeof(treeNode*));
    treeNode* pRoot = tree_allocate(nodepool, sizeof(treeNode));
    object* rootbuf = tree_allocate(nodepool, sizeof(treeNode)*OBJBUFSIZE);
    *pRoot = (treeNode){
        .buf = rootbuf,
        .places = ~0UL,
    };
    *pMetadataStruct = (objTree){
        .objectAllocPool = objectpool,
        .nodeAllocPool = nodepool,
        .searchbuf = searchbuf,
        .searchbufsize = SEARCHBUFSIZE,
        .root = pRoot
    };
    return pMetadataStruct;
}
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
    printf("added up all the coordinates\n");
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
    printf("%p %ld\n", currentNode, currentNode->split.isx);
    printf("got through ascent phase\n");
    if(currentNode->up){
        if(!currentNode->up->split.isx) currentNode->split.isx = 1;
    }
    else currentNode->split.isx = 1;
    
    printf("checked for axis of split");
    currentNode->left = tree_allocate(tree->objectAllocPool, sizeof(treeNode));
    currentNode->right = tree_allocate(tree->objectAllocPool, sizeof(treeNode));
    fprintf(stderr, "allocated nodes\n");
    currentNode->left->buf = tree_allocate(tree->nodeAllocPool, sizeof(object)*OBJBUFSIZE);
    currentNode->right->buf = tree_allocate(tree->nodeAllocPool, sizeof(object)*OBJBUFSIZE);    
    fprintf(stderr, "allocated buffers\n");
    tree_balanceBuffers(currentNode, currentNode->left, currentNode->right);
    tree_free(currentNode->buf);
    tree->bufCount++;
    return 0;
}
/*
111111111100000000000 - places
&
000000000011111111111 - mask

000000000000000000000


[****|**--|**--|--------------------------------]

*/