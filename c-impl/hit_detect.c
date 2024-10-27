#include "hit_detect.h"
#define SQUARE(x) x*x
//recursive function for hit detection
static int hit_flagObjects_aux(treeNode* node){
    if(!node->buf){
        hit_flagObjects_aux(node->left);
        hit_flagObjects_aux(node->right);
    }
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        if(!node->buf[i].s && !node->buf[i].hit){ //if size is zero then the object is either uninitialized or does not interact physically
            for(unsigned j = 0; j < OBJBUFSIZE; j++){ //check inside the same buffer
                if(j == i) continue; //dont wanna check with ourselves
                if(node->buf[j].hit) continue; //already intersecting with something
                double dist = sqrt(SQUARE(node->buf[j].x - node->buf[i].x) + SQUARE(node->buf[j].y - node->buf[i].y));
                if(dist < (node->buf[i].s + node->buf[j].s)){
                    node->buf[i].hit = node->buf + j;
                    node->buf[j].hit = node->buf + i;
                }
            }
            
        }
    }
    return 0;
}
//runner function
int hit_flagObjects(objTree* tree){
    return hit_flagObjects_aux(tree->root);
}
/*
Return all the buffers containing the area specified by `rect`.
Expects you to provide a buffer of at least 12 * sizeof(object*)
Returns the number of buffers found. The number ranges from 0 to 12.
Traverses from the top of the tree - when it's shallower than 8 levels,
its beter than the deep version.
Query is not required, its just for compatibility with 
*/
int hit_findRect_shallow(objTree* tree, treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static 12]){
    unsigned numWritten = 0;
    
}
int hit_findRect_shallow_aux(treeNode* node __attribute__((unused)), rect_ofex rect, object* results[static 12]){
    
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
int tree_findRect_deep(objTree* tree, treeNode* node, rect_ofex query, object* results[static 12]){

}