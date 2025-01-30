#include "tree_alloc.h"
#include <string.h>
#include <stdlib.h>

tree_allocPool tree_allocInitPool(size_t size){
    tree_allocPool retPtr = calloc(sizeof(struct tree_allocPool_s), 1);
    retPtr->subpools[0].pStart = calloc(1, size),
    retPtr->subpools[0].allocated = size;
    retPtr->holes = calloc(1, 100*sizeof(free_hole));
    retPtr->holesallocated = 100;
    return retPtr;
}
void tree_allocDestroyPool(tree_allocPool pool){
    for(unsigned i = 0; i < pool->currentSubpool + 1; i++){
        free(pool->subpools[i].pStart);
    }
    free(pool);
}

void* __attribute__((malloc)) tree_allocate(tree_allocPool pool, size_t size) {
    tree_allocSubpool* subpool = &pool->subpools[pool->currentSubpool];
    if(pool->holecount){
        for(unsigned i = pool->holecount; i > 0; i--){
            if(pool->holes[i].size == size){
                void* retPtr =  pool->holes[i].ptr;
                pool->holes[i] = (free_hole){ 0 };
                pool->holecount--;
                return retPtr; 
            }
            else if(pool->holes[i].size > size){
                void* retPtr =  pool->holes[i].ptr;
                pool->holes[i] = (free_hole){
                    .ptr = retPtr + size,
                    .size = pool->holes[i].size - size};
                pool->holecount--;
                return retPtr;
            }
        }
    }
    if (subpool->allocated - subpool->used < size) {
        pool->currentSubpool++;
        pool->subpools[pool->currentSubpool] = (tree_allocSubpool){
            .pStart = calloc(subpool->allocated*2, 1),
            .allocated = subpool->allocated*2,
        };
        subpool = &pool->subpools[pool->currentSubpool];
    }

    void* retptr = subpool->pStart + subpool->used;
    subpool->used += size;
    return retptr;
}
void tree_free(void* pointer, size_t size, tree_allocPool pool){
    if(pool->holecount == pool->holesallocated){
        pool->holesallocated *= 1.5;
        pool->holes = realloc(pool->holes, pool->holesallocated*1.5);
    }
    pool->holes[pool->holecount] = (free_hole){
        .ptr = pointer,
        .size = size,
    };
    memset(pointer, '\0', size);
};