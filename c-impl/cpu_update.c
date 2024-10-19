#include <stdlib.h>
#include <stdio.h>
#include "cpu_update.h"
int vector_update(obj_arr* arr, unsigned size){
    for(unsigned i = 0; i < size; i++){
        for(unsigned j = 0; j < arr[i].size; j++){
            object* obj = arr[i].arr + j;
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
            
        }
    }
}
int scalar_update(obj_arr* arr, unsigned size){
    for(unsigned i = 0; i < size; i++){
        for(unsigned j = 0; j < arr[i].size; j++){
            object* obj = arr[i].arr + j;
            obj->x += obj->v.x;
            obj->y += obj->v.y;
        }
    }
}

int main(void){
    
}