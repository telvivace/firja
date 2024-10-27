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