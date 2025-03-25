#include <stdio.h>
#include <string.h>
#include "objtree.h"
#include "treeutils.h"
#include "settings.h"
#include "liblogs.h"
#define NFLAG_IN_OPTQUEUE (uint16_t)1
objTree* tree_initTree(){
    orb_logf(PRIORITY_DBUG,"init pool 1");
    tree_allocPool objectpool = tree_allocInitPool(1 * 1e6); //one million (1 MB)
    orb_logf(PRIORITY_DBUG,"init pool 2");
    tree_allocPool nodepool = tree_allocInitPool(1 * 1e3); //one thousand (1 kB)
    orb_logf(PRIORITY_DBUG,"alloc metadata");
    objTree* pMetadataStruct = tree_allocate(nodepool, sizeof(objTree));
    orb_logf(PRIORITY_DBUG,"alloc searchbuf");
    object** searchbuf = tree_allocate(nodepool, SEARCHBUFSIZE*sizeof(object*));
    orb_logf(PRIORITY_DBUG,"alloc root node");
    treeNode* pRoot = tree_allocate(nodepool, sizeof(treeNode));
    orb_logf(PRIORITY_DBUG,"alloc root node buf");
    object* rootbuf = tree_allocate(objectpool, sizeof(treeNode)*OBJBUFSIZE);
    *pRoot = (treeNode){
        .buf = rootbuf,
        .places = ~0UL,
        .split.isx = 1,
        .bindrect = (rect_llhh){
            .lowlow = (point){.x = g_leftborder - 200, .y = g_bottomborder - 200},
            .highhigh = (point){.x = g_rightborder + 200, .y = g_topborder + 200}
        }
    };
    *pMetadataStruct = (objTree){
        .searchbuf = searchbuf,
        .searchbufsize = SEARCHBUFSIZE,
        .root = pRoot,
        .bufCount = 1, //root node
        .opt_queue = (treeOptimizationQueue){
            .allocated = 100,
            .nodecount = 0,
            .nodes = calloc(1, 100*sizeof(treeNode*))
        }
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
                break;
            default:
                orb_log(PRIORITY_ERR, "what the hell? we got a 3rd dimension! it's %u", currentNode->split.isx);
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
    orb_logf(PRIORITY_DBUG,"==================\ntree_balanceBuffers2:");
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
        tree_printObject(PRIORITY_TRACE, parent->buf + i);
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
    orb_logf(PRIORITY_TRACE,"comparison: %lx", OBJBUFFULLMASK);
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
    parentNode->places &= ~(1UL << __builtin_ctzl(parentNode->places)); //mark as occupied
    return 0;
}
int tree_optimizeNodes(objTree* tree){
    for(unsigned i = 0; i < tree->opt_queue.nodecount; i++){
        if(!tree->opt_queue.nodes[i]) continue;
        treeNode* node1 = tree->opt_queue.nodes[i];
        unsigned numObjects1 = __builtin_popcountl(~(node1->places)); //count the number of zeros
        if(numObjects1 > ((unsigned)OBJBUFSIZE / 4U)){
            //remove from queue
            node1->flags &= ~(NFLAG_IN_OPTQUEUE);
            tree->opt_queue.nodes[i] = (treeNode*)0; 
            tree->opt_queue.nodecount--;
            continue;
        }
        treeNode* node2;
        if(node1->up->left == node1) node2 = node1->up->right;
        else node2 = node1->up->left;
        if(!node2->buf || !node1->buf) continue; //we can't merge with a non-leaf node
        unsigned numObjects2 = __builtin_popcountl(~(node2->places));
        if((numObjects1 + numObjects2) <= 2*OBJBUFSIZE/3){
            node1->flags &= ~(NFLAG_IN_OPTQUEUE);
            //orb_log(PRIORITY_WARN, "Optimizing nodes.");
            treeNode* src;
            treeNode* dest;
            #if DBUG_MODE == 1
            unsigned realcountL = 0;
            unsigned realcountR = 0;
            for(unsigned i = 0; i < OBJBUFSIZE; i++){
                if(node1->up->left->buf[i].s) realcountL++;
                if(node1->up->right->buf[i].s) realcountR++;
            }
            #endif
            unsigned numObjects_src = 0;
            if(numObjects1 > numObjects2){
                src = node2;
                dest = node1;
                numObjects_src = numObjects2;
                
            }
            else{
                src = node1;
                dest = node2;
                numObjects_src = numObjects1;
            }
            unsigned srcoffset;
            unsigned destoffset;
            for(unsigned j = 0; j < numObjects_src; j++){
                srcoffset =  __builtin_ctzl(~(src->places));
                destoffset = __builtin_ctzl(dest->places);
                memcpy(dest->buf + destoffset, src->buf + srcoffset, sizeof(object));
                dest->places &= ~(1UL << destoffset);
                src->places |= (1UL << srcoffset);
            }
            tree->opt_queue.nodes[i] = (treeNode*)0; 
            //for future use, because balanceBuffers expects it to be calloc'd
            memset(src->buf, '\0', OBJBUFSIZE*sizeof(object));
            node1->flags &= ~(NFLAG_IN_OPTQUEUE);
            dest->up->places = dest->places;
            dest->up->buf = dest->buf;
            
            #if DBUG_MODE == 1
            //check if the number of objects in both buffers is correct
            unsigned papercountP = __builtin_popcountl(~(node1->up->places));
            unsigned realcountP = 0;
            for(unsigned k = 0; k < OBJBUFSIZE; k++){
                if(node1->up->buf[k].s) realcountP++;
            }
            if(realcountL + realcountR != papercountP || realcountL + realcountR != realcountP){
                orb_log(PRIORITY_WARN, "OptimizeNodes: Object count mismatch detected!");
                orb_logf(PRIORITY_WARN, "Count on paper: %d(P). Real count: %d(P), was %d(L), %d(R).", papercountP, realcountP, realcountL, realcountR);
                orb_logf(PRIORITY_WARN, "parent buffer: %p", dest->up->buf);
                orb_logf(PRIORITY_WARN, "src places: %lx", src->places);
                orb_logf(PRIORITY_WARN, "Parent node object count / sum of two children: %u/%u, %lx ",  __builtin_popcountl(~(node1->up->places)), numObjects1 + numObjects2, node1->up->places);
                orb_logf(PRIORITY_WARN, "Parent buffer:");
                tree_printBufferContents(PRIORITY_WARN, node1->up->buf);
            }

            #endif
            dest->places = ~0UL;
            dest->buf = (void*)0;
            tree->opt_queue.nodecount--;
        }
    }
    return 0;
}
int tree_OptqueueSubmit(objTree* tree, treeNode* node){
    if(tree->opt_queue.allocated < tree->opt_queue.nodecount + 1){
        size_t new_alloc = tree->opt_queue.allocated * 1.5;
        treeNode** new_nodes = realloc(tree->opt_queue.nodes, new_alloc * sizeof(treeNode*));
        if (!new_nodes) {
            orb_logf(PRIORITY_ERR, "Memory allocation failed in tree_OptqueueSubmit");
            return -1; // or handle gracefully
        }
        memset(new_nodes + tree->opt_queue.allocated, 0, 
               (new_alloc - tree->opt_queue.allocated) * sizeof(treeNode*));
        tree->opt_queue.nodes = new_nodes;
        tree->opt_queue.allocated = new_alloc;
        
    }
    node->flags |= NFLAG_IN_OPTQUEUE;
    //fill in any holes that may have been created
    for(unsigned i = 0; i < tree->opt_queue.nodecount; i++){
        if(tree->opt_queue.nodes[i] == 0){
            tree->opt_queue.nodes[i] = node;
            tree->opt_queue.nodecount++;
            return 0;
        }
    }
    //if the loop hasn't returned, it's a continuous block so we can just write
    tree->opt_queue.nodes[tree->opt_queue.nodecount] = node;
    tree->opt_queue.nodecount++;
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
    if(!node->left){
        node->left = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
        *(node->left) = (treeNode){
            .buf = node->buf, //reuse parent buf. Only works with balanceBuffers2
            .up = node,
            .places = ~0UL,
            .level = node->level + 1,
        };
    }
    if(!node->right){
        tree->bufCount++;
        node->right = tree_allocate(tree->nodeAllocPool, sizeof(treeNode));
        *(node->right) = (treeNode){
            .buf = tree_allocate(tree->objectAllocPool, sizeof(object)*OBJBUFSIZE),
            .up = node,
            .places = ~0UL,
            .level = node->level + 1,
        };
    }
    //we are left with two child nodes, of which one has a buffer and one doesn't.
    node->left->bindrect = (rect_llhh){
                .lowlow = node->bindrect.lowlow,
                .highhigh = node->split.isx ? 
                    (point){ .x = node->split.value, .y = node->bindrect.highhigh.y}
                    :
                    (point){ .x = node->bindrect.highhigh.x, .y = node->split.value}
            };
    node->right->bindrect = (rect_llhh){
                .lowlow = node->split.isx ? 
                    (point){ .x = node->split.value, .y = node->bindrect.lowlow.y}
                    :
                    (point){ .x = node->bindrect.lowlow.x, .y = node->split.value},
                .highhigh = node->bindrect.highhigh
            };
    //one may have a null buffer if it was merged before by the tree optimization function
    if(!node->left->buf) node->left->buf = node->buf;
    else if(!node->right->buf) node->right->buf = node->buf;
    //we are left with two children, one has its own buffer and one has a duplicated parent buffer.
    
    orb_logf(PRIORITY_TRACE,"Parent node has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
    orb_logf(PRIORITY_TRACE,"Left child has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->left->bindrect.lowlow.x, node->left->bindrect.highhigh.x, node->left->bindrect.lowlow.y, node->left->bindrect.highhigh.y);
    orb_logf(PRIORITY_TRACE,"Right child has a rectangle of x: %lf -- %lf, y: %lf -- %lf ", node->right->bindrect.lowlow.x, node->right->bindrect.highhigh.x, node->right->bindrect.lowlow.y, node->right->bindrect.highhigh.y);

    orb_logf(PRIORITY_DBUG, "allocated buffers");
    tree_balanceBuffers2(node, node->left, node->right);
    #if DBUG_MODE == 1
    //check if the number of objects in both buffers is correct
    unsigned realcountL = 0;
    unsigned realcountR = 0;
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        if(node->left->buf[i].s) realcountL++;
        if(node->right->buf[i].s) realcountR++;
    }
    unsigned papercountL = 0;
    unsigned papercountR = 0;
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        if(!(node->left->places & 1UL << i)) papercountL++;
        if(!(node->right->places & 1UL << i)) papercountR++;
    }
    if(papercountL != realcountL || papercountR != realcountR || (papercountL + papercountR) != OBJBUFSIZE){
        orb_log(PRIORITY_WARN, "Object count mismatch detected!");
        orb_logf(PRIORITY_WARN, "Count on paper: %d(L) + %d(R). Real count: %d(L) + %d(R). Both supposed to be %d(L+R).", papercountL, papercountR, realcountL, realcountR, OBJBUFSIZE);
    }

    #endif
    
    node->buf = (void*)0;
    return 0;
}