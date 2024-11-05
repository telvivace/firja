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
        .split.isx = 1,
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
                    printf("descend left\n");
                    currentNode = currentNode->left;
                }
                else {
                    printf("descend right\n");
                    currentNode = currentNode->right;
                }
                break;
            case 2:
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
    object* dest = (void*)0;
    if(parent->buf == left_child->buf){ 
        dest = right_child->buf;
        printf("left child buf == parent buf \n");
    }
    else{
        dest = left_child->buf;
        printf("right child buf == parent buf \n");
    }  
    printf("==================\ntree_balanceBuffers2:\n");
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
    for(unsigned i = 0; i < OBJBUFSIZE; i++){

    }
    parent->split.value = average;
    
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
            average
        );
        //first value is the x or y field of the i-th struct in buf
        if(*(double*)( (char*)&(parent->buf[i]) + coordoffset ) > average){ 
            memcpy(dest + destwritten, parent->buf + i, sizeof(object));
            memset(parent->buf + i, '\0', sizeof(object));      //wipe the object
            parent->places |= 1UL << i;                         //mark as vacant
        }
    }
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
    parentNode->places &= ~(1 << __builtin_ctzl(parentNode->places)); //set the bit to 0
    return 0;
}

int tree_splitNode(objTree* tree, treeNode* node){
    printf("==================\ntree_splitNode:\n");
    printf("node pointer: %p split coordinate: %c\n", node, node->split.isx ? 'x' : 'y');
    printf("got through ascent phase\n");
    if(node->up){
        if(!node->up->split.isx) node->split.isx = 1;
    }
    else node->split.isx = 1;
    
    printf("checked for axis of split. allocating nodes\n");
    node->left = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    node->right = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
    fprintf(stderr, "allocated nodes. allocating buffers\n");
    *(node->left) = (treeNode){
        .buf = node->buf, //reuse parent buf. Only works w/ balamceBuffers2
        .up = node,
        .places = ~0UL,
    };
    *(node->right) = (treeNode){
        .buf = tree_allocate(tree->objectAllocPool, sizeof(object)*OBJBUFSIZE),
        .up = node,
        .places = ~0UL,
    };

    fprintf(stderr, "allocated buffers\n");
    tree_balanceBuffers2(node, node->left, node->right);
    tree_free(node->buf);
    node->buf = (void*)0;
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