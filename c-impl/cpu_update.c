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

/*
let v_cm = (
    //x velocity component
    (one.m as f32 * one.v.0 + other.m as f32 * other.v.0)/(one.m + other.m) as f32,
    //y velocity component
    (one.m as f32 * one.v.1 + other.m as f32 * other.v.1)/(one.m + other.m) as f32
);
let one_updated = object {
    v : (
        //reverse relative velocity in respect to cm
        (one.v.0 - v_cm.0)*-1.0 + v_cm.0,
        (one.v.1 - v_cm.1)*-1.0 + v_cm.1
        ),
        ..*one
};
let other_updated = object {
    v : (
        //reverse relative velocity in respect to cm
        (other.v.0 - v_cm.0)*-1.0 + v_cm.0,
        (other.v.1 - v_cm.1)*-1.0 + v_cm.1
    ),
    ..*other
};
*/
int main(void){
    
}