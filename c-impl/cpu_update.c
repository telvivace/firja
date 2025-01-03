#include <stdlib.h>
#include <stdio.h>
#include "cpu_update.h"
#include "objtree.h"
#include "settings.h"

int vector_update_aux(treeNode* node){
    printf("At node: %p\n", node);
    if(!node->buf){
        printf("buffer is null. Left: %p, Right: %p\n", node->left, node->right);
        vector_update_aux(node->left);
        vector_update_aux(node->right);
        printf("UP ( %p -> %p)\n", node, node->up);
        return 0;
    }
    printf("buffer is valid: %p\n", node->buf);
    printf("\nchecking no. ");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        printf("%d..", i);
        object* obj = node->buf + i;
        if(!obj){
            printf("OBJECT NUMBER %d IS NULL!\n", i);
            continue;
        }
        if(obj->hit) {
            speed v_cm = {
                .x = (obj->m * obj->v.x + obj->hit->m * obj->hit->v.x)/(obj->m + obj->hit->m),
                .y = (obj->m * obj->v.y + obj->hit->m * obj->hit->v.y)/(obj->m + obj->hit->m)
            };
            obj->v = (speed){
                .x = (obj->v.x - v_cm.x)*-1 + v_cm.x,
                .y = (obj->v.y - v_cm.y)*-1 + v_cm.y,
            };
            obj->hit->v = (speed){
                .x = (obj->hit->v.x - v_cm.x)*-1 + v_cm.x,
                .y = (obj->hit->v.y - v_cm.y)*-1 + v_cm.y,
            };
            //TODO WARN naive?
            obj->hit->hit = 0;
            obj->hit = 0;
        }
        //hardcoded borders of a box so that things don't just fly apart
        if(obj->x > RIGHTBORDER || obj->x < LEFTBORDER) obj->v.x *= -1;
        if(obj->y > TOPBORDER || obj->y < BOTTOMBORDER) obj->v.x *= -1;
    }
    putc('\n', stdout);
    putc('\n', stdout);
    return 0;
}
int vector_update(objTree* tree){
    printf("Vector update.\n");
    return vector_update_aux(tree->root);
}

int scalar_update_aux(treeNode* node){
    if(!node->buf){
        printf("buffer is null. Left: %p, Right: %p\n", node->left, node->right);
        printf("going left\n");
        scalar_update_aux(node->left);
        printf("going right\n");
        scalar_update_aux(node->right);
        printf("UP ( %p -> %p)\n", node, node->up);
        return 0;
    }
    printf("\nchecking no. ");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        printf("%d..", i);
        object* obj = node->buf + i;    
        obj->x += obj->v.x;
        obj->y += obj->v.y;
    }
    printf("\n\n");
    printf("UP ( %p -> %p)\n", node, node->up);
    return 0;
}
int scalar_update(objTree* tree){
    printf("Scalar update.\n");
    return scalar_update_aux(tree->root);
}