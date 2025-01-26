#include "hit_detect.h"
#include <tgmath.h>
#include <stdio.h>
#include "liblogs.h"
#define SQUARE(x) ((x)*(x))

//documentation in the runner function below
static void hit_findRect_shallow_aux(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static SEARCHBUFSIZE], unsigned* numWritten){
    if(!node->buf){
        if(node->split.isx) {
            if(rect.offset.x + rect.extent.width < node->split.value){
                hit_findRect_shallow_aux(tree, node->left, rect, results, numWritten);
            }
            else if (rect.offset.x + rect.extent.width >= node->split.value && rect.offset.x <= node->split.value){
                hit_findRect_shallow_aux(tree, node->left, rect, results, numWritten);
                hit_findRect_shallow_aux(tree, node->right, rect, results, numWritten);
            }
            else {
                hit_findRect_shallow_aux(tree, node->right, rect, results, numWritten);
            }
        }
        else {
            if(rect.offset.y + rect.extent.height < node->split.value){
                hit_findRect_shallow_aux(tree, node->left, rect, results, numWritten);
            }
            else if (rect.offset.y + rect.extent.height >= node->split.value && rect.offset.y <= node->split.value){
                hit_findRect_shallow_aux(tree, node->left, rect, results, numWritten);
                hit_findRect_shallow_aux(tree, node->right, rect, results, numWritten);
            }
            else {
                hit_findRect_shallow_aux(tree, node->right, rect, results, numWritten);
            }
        }
    }
    else {
        results[*numWritten] = node->buf;
        *numWritten += 1;
    }
}
/*
Return all the buffers containing the area specified by `rect`.
Expects you to provide a buffer of at least 12 * `sizeof(object*)`
Returns the number of buffers found. The number ranges from 0 to 12.
Traverses from the top of the tree - when it's shallower than 8 levels,
its beter than the deep version.
`node` is not required, its just for compatibility with the deep version
*/
int hit_findRect_shallow(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static SEARCHBUFSIZE]){
    unsigned numWritten = 0;
    hit_findRect_shallow_aux(tree, node, rect, results, &numWritten);
    return numWritten;
}

/*
Return all the buffers containing the area specified by `rect`.
Expects you to provide a buffer of at least 12 * sizeof(object*)
Returns the number of buffers found. The number ranges from 0 to 12.
Goes up the tree, until it proves that there are no more splits upstream.
Then it descends in the same way as shallow. Efficient when faced with
deep trees (at least 8 levels). 2x less efficient than the shallow version
if the tree is <4 levels deep, then it slowly tips in favour of the deep version
*/
int tree_findRect_deep(objTree* tree, treeNode* node, rect_ofex query, object* results[static SEARCHBUFSIZE]){
    return 0;
}
//recursive function for hit detection
static int hit_flagObjects_aux(objTree* tree, treeNode* node){
    if(!node->buf){
        orb_logf(PRIORITY_TRACE,"descending from node %p down into %p (left) and %p (right)", node, node->left, node->right);
        hit_flagObjects_aux(tree, node->left);
        hit_flagObjects_aux(tree, node->right);
        return 0;
    }
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        orb_logf(PRIORITY_TRACE,"Structure no. %d has fields:"\
               "x: %lf y: %lf\n"\
               "s: %f m: %d\n"\
               "hit pointer: %p", i,  node->buf[i].x, node->buf[i].y, node->buf[i].s, node->buf[i].m, node->buf[i].hit);
        if(node->buf[i].s && !node->buf[i].hit){ //if size is zero then the object is either uninitialized or does not interact physically
            orb_logf(PRIORITY_TRACE,"Non-collided object detected:\n\tObject no. %d is valid", i); 
            
            //search for buffers that contain the object (node->buf[i])'s area of influence
            unsigned numResults = hit_findRect_shallow(
                tree,
                node, 
                (rect_ofex){ //rectangle around the object
                    .offset = (point){
                        .x = node->buf[i].x - (node->buf[i].s + MAXOBJSIZE),
                        .y = node->buf[i].y - (node->buf[i].s + MAXOBJSIZE),
                    },
                    .extent = (extent){
                        .width = 2*(node->buf[i].s + MAXOBJSIZE),
                        .height = 2*(node->buf[i].s + MAXOBJSIZE)
                    }
                },
                tree->searchbuf
            );
            if(numResults > (SEARCHBUFSIZE - 4))
                orb_log(PRIORITY_WARN, "Approaching searchbuf size limit: %d/%d", numResults, SEARCHBUFSIZE);
            //do hit detection with the buffers found
            for(unsigned bufindex = 0; bufindex < numResults; bufindex++){
                object* buf = tree->searchbuf[bufindex];
                for(unsigned j = 0; j < OBJBUFSIZE; j++){
                    if(buf + j == node->buf + i) continue; //shouldnt hit itself
                    if(buf[j].s && !buf[j].hit){
                        double dist = sqrt(SQUARE(buf[j].x - node->buf[i].x) + SQUARE(buf[j].y - node->buf[i].y));
                        orb_logf(PRIORITY_TRACE,"\tDistance between objects %d and %d is %lf", i, j, dist);
                        
                        if(dist < (node->buf[i].s + buf[j].s)){
                            node->buf[i].hit = buf + j;
                            buf[j].hit       = node->buf + i;
                            orb_logf(PRIORITY_TRACE,"\tObject no. %d in buffer1 intersects with object no. %d in buffer%d", i, bufindex, j);
                            break;
                        }
                    }
                }
            }
        }
    }
    orb_logf(PRIORITY_DBUG,"Moving up from node %p to node %p\n", node, node->up);
    return 0;
}
//runner function
int hit_flagObjects(objTree* tree){
    orb_logf(PRIORITY_DBUG,"==============\nhit_flagObjects");
    hit_flagObjects_aux(tree, tree->root);
    orb_logf(PRIORITY_DBUG,"aux returned, returning");
    return 0;
}