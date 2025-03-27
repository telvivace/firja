#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu_update.h"
#include "objtree.h"
#include "settings.h"
#include "liblogs.h"
#define VECTORUPDATE_SEENBEFORE 1U
#define NFLAG_IN_OPTQUEUE 1

int vector_update_aux(objTree* tree, treeNode* node){
    orb_logf(PRIORITY_TRACE,"At node: %p", node);
    if(!node->buf){
        nullbuf:
        orb_logf(PRIORITY_TRACE,"buffer is null. Left: %p, Right: %p", node->left, node->right);
        vector_update_aux(tree, node->left);
        vector_update_aux(tree, node->right);
        orb_logf(PRIORITY_TRACE,"UP ( %p -> %p)", node, node->up);
        return 0;
    }
    #if RECYCLE == 1
    unsigned numObjects =__builtin_popcountl(~(node->places)); //count the number of zeros
    if(numObjects <= ((unsigned)OBJBUFSIZE / 4U)){
        orb_logf(PRIORITY_DBUG, "Node's count is %u/%u", numObjects, OBJBUFSIZE);
        if(!(node->flags & NFLAG_IN_OPTQUEUE)){
            tree_OptqueueSubmit(tree, node);
        }
    }
    #endif
    orb_logf(PRIORITY_TRACE,"buffer is valid: %p", node->buf);
    orb_logf(PRIORITY_BLANK,"checking no. ");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        orb_logf(PRIORITY_BLANK,"%d..", i);
        object* obj = node->buf + i;
        if(obj->s){

            //one day code will be here

            if(obj->hit) {
                orb_logf(PRIORITY_TRACE,"object velocity is x:%f y:%f", obj->v.x, obj->v.y);
                speed v_cm = {
                    .x = (obj->m * obj->v.x + obj->hit->m * obj->hit->v.x)/(obj->m + obj->hit->m),
                    .y = (obj->m * obj->v.y + obj->hit->m * obj->hit->v.y)/(obj->m + obj->hit->m)
                };
                obj->v = (speed){
                    .x = (obj->v.x - v_cm.x)*-1 + v_cm.x,
                    .y = (obj->v.y - v_cm.y)*-1 + v_cm.y,
                };
                orb_logf(PRIORITY_TRACE,"setting object velocity to x:%f y:%f", obj->v.x, obj->v.y);
                obj->hit->v = (speed){
                    .x = (obj->hit->v.x - v_cm.x)*-1 + v_cm.x,
                    .y = (obj->hit->v.y - v_cm.y)*-1 + v_cm.y,
                };



                obj->hit->hit = 0;
                obj->hit = 0;
            }
            obj->v.y += GRAVITY; //because SDL2 has 0,0 on the top
            //hardcoded borders of a box so that things don't just fly apart
            if((obj->x > g_rightborder && obj->v.x > 0) || (obj->x < g_leftborder && obj->v.x < 0)) obj->v.x *= -BOUNCE;
            if((obj->y > g_topborder && obj->v.y > 0) || (obj->y < g_bottomborder && obj->v.y < 0)) obj->v.y *= -BOUNCE;
            if(obj->s){
                if(!(obj->flags & VECTORUPDATE_SEENBEFORE)){
                    tree->validObjCount++;
                    obj->flags |= VECTORUPDATE_SEENBEFORE;
                }
                if((obj->x > node->bindrect.highhigh.x || obj->x < node->bindrect.lowlow.x || obj->y > node->bindrect.highhigh.y || obj->y < node->bindrect.lowlow.y)){
                    orb_logf(PRIORITY_TRACE,"(%lf, %lf) is outside (%lf, %lf) x (%lf, %lf)", obj->x, obj->y, node->bindrect.lowlow.x, node->bindrect.lowlow.y, node->bindrect.highhigh.x, node->bindrect.highhigh.y);
                    tree_insertObject(tree, obj); //can also split, rendering this function invalid! (insane edge case, i've run 10 million cycles with 1000 objects and it didnt happen)
                    //it happened when I was running 10 frames with a 1000 objects and rand() decided to just do that
                    if(__builtin_expect_with_probability (!node->buf, 0, .99)){ //so this fix is required
                        tree->relocations++;
                        goto nullbuf;
                    }
                    memset(obj, '\0', sizeof(object));
                    node->places |= 1UL << i; //mark this place as vacant
                    tree->relocations++;
                }
            }
        }
        
    }
    orb_logf(PRIORITY_TRACE, "\n");
    return 0;
}
int vector_update(objTree* tree){
    orb_logf(PRIORITY_DBUG,"Vector update.");
    return vector_update_aux(tree, tree->root);
}

int scalar_update_aux(treeNode* node){
    if(!node->buf){
        orb_logf(PRIORITY_TRACE,"buffer is null. Left: %p, Right: %p", node->left, node->right);
        orb_logf(PRIORITY_DBUG,"going left");
        scalar_update_aux(node->left);
        orb_logf(PRIORITY_DBUG,"going right");
        scalar_update_aux(node->right);
        orb_logf(PRIORITY_DBUG,"UP ( %p -> %p)", node, node->up);
        return 0;
    }
    orb_logf(PRIORITY_BLANK,"checking no. ");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        orb_logf(PRIORITY_BLANK,"%d..", i);
        object* obj = node->buf + i;
        obj->flags = 0;   
        obj->x += obj->v.x;
        obj->y += obj->v.y;
    }
    orb_logf(PRIORITY_BLANK,"\n\n");
    orb_logf(PRIORITY_DBUG,"UP ( %p -> %p)", node, node->up);
    return 0;
}
int scalar_update(objTree* tree){
    orb_logf(PRIORITY_DBUG,"Scalar update.");
    return scalar_update_aux(tree->root);
}