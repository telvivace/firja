#include "hit_detect.h"
#include <tgmath.h>
#include <stdio.h>
#include "liblogs.h"
#define SQUARE(x) ((x)*(x))

//documentation in the runner function below
static void tree_findRect_shallow_aux(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static SEARCHBUFSIZE], unsigned* pNumWritten){
    if(!node->buf){
        if(node->split.isx) {
            if(rect.offset.x + rect.extent.width < node->split.value){
                tree_findRect_shallow_aux(tree, node->left, rect, results, pNumWritten);
            }
            else if (rect.offset.x + rect.extent.width >= node->split.value && rect.offset.x <= node->split.value){
                tree_findRect_shallow_aux(tree, node->left, rect, results, pNumWritten);
                tree_findRect_shallow_aux(tree, node->right, rect, results, pNumWritten);
            }
            else {
                tree_findRect_shallow_aux(tree, node->right, rect, results, pNumWritten);
            }
        }
        else {
            if(rect.offset.y + rect.extent.height < node->split.value){
                tree_findRect_shallow_aux(tree, node->left, rect, results, pNumWritten);
            }
            else if (rect.offset.y + rect.extent.height >= node->split.value && rect.offset.y <= node->split.value){
                tree_findRect_shallow_aux(tree, node->left, rect, results, pNumWritten);
                tree_findRect_shallow_aux(tree, node->right, rect, results, pNumWritten);
            }
            else {
                tree_findRect_shallow_aux(tree, node->right, rect, results, pNumWritten);
            }
        }
    }
    else {
        if(*pNumWritten < SEARCHBUFSIZE){
            results[*pNumWritten] = node->buf;
            *pNumWritten += 1;
        }
        else{
            orb_logf(PRIORITY_ERR, "INSUFFICIENT SPACE FOR RETURNING THE RANGE QUERY");
        }
    }
}
/*
Return all the buffers containing the area specified by `rect`.
Expects a buffer of at least 12 * `sizeof(object*)`.
Returns the number of buffers found. The number ranges from 0 to 12.
Traverses from the top of the tree - when it's shallower than 8 levels,
its beter than the deep version.
`node` is not required, its just for compatibility with the deep version
*/
int tree_findRect_shallow(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static SEARCHBUFSIZE]){
    unsigned numWritten = 0;
    tree_findRect_shallow_aux(tree, node, rect, results, &numWritten);
    return numWritten;
}

/*
Return all the buffers containing the area specified by `rect`.
Expects you to provide a buffer of at least 12 * sizeof(object*)
and the node you are querying from, as it's a local search.
Returns the number of buffers found. The number ranges from 0 to 12.
Goes up the tree, until it proves that the query does not split upstream.
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
               "hit pointer: %p", i,  node->buf[i].x, node->buf[i].y, node->buf[i].s, node->buf[i].m, node->buf[i].hitgroup);
        if(node->buf[i].s){ //if size is zero then the object is either uninitialized or does not interact physically
            orb_logf(PRIORITY_TRACE,"Non-collided object detected:\n\tObject no. %d is valid", i); 
            
            //search for buffers that contain the object (node->buf[i])'s area of influence
            unsigned numResults = tree_findRect_shallow(
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
            unsigned hitscount = 0;
            //do hit detection with the buffers found
            for(unsigned bufindex = 0; bufindex < numResults; bufindex++){
                object* buf = tree->searchbuf[bufindex];
                for(unsigned j = 0; j < OBJBUFSIZE; j++){
                    if(buf + j == node->buf + i) continue; //shouldnt hit itself
                    if(buf[j].s){
                        double dist = SQUARE(buf[j].x - node->buf[i].x) + SQUARE(buf[j].y - node->buf[i].y);
                        orb_logf(PRIORITY_TRACE,"\tDistance between objects %d and %d is %lf", i, j, dist);
                        
                        if(dist < SQUARE(node->buf[i].s + buf[j].s)){
                            hitscount++;

                            if(buf[j].hitgroup){
                                
                                //check if node->buf[i] is already in a collision with buf[j] 
                                //it starts at 1 because the first object in the hitgroup is buf[i]
                                for(unsigned k = 1; k < buf[j].hitgroup->length; k++){
                                    if(*(tree->hitObjectQueue.objects + buf[j].hitgroup->start + k) == node->buf + i){
                                        continue;//ensures that a pair of objects is processed only once!
                                    }
                                }
                            }
                            
                            if(hitscount == 1){
                                tree->hitObjectQueue.objects[tree->hitObjectQueue.objcount] = node->buf + i;
                                tree->hitGroupQueue.groups[tree->hitGroupQueue.groupcount] = (hitGroup){
                                    .start = tree->hitObjectQueue.objcount, 
                                    .length = 1,
                                };
                                tree->hitObjectQueue.objcount++;
                            }
                            tree->hitObjectQueue.objects[tree->hitObjectQueue.objcount] = buf + j;
                            tree->hitObjectQueue.objcount++;

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