#include "tree_alloc.h"
#include <stdlib.h>
struct tree_allocPool{
    size_t basesize;
    unsigned currentSubpool; //the subpool currently being filled
    tree_allocSubpool* subpools[34];// ensures > 16GB space even with tiny pools
                                    // (exponentially^2 larger pools allocated)
};
struct tree_allocSubpool{
    void* pStart;
    size_t allocated;
    size_t used;
};
tree_allocPool tree_allocInitPool(size_t size){
    tree_allocPool retPtr = calloc(sizeof(struct tree_allocPool_s), 1);
    retPtr->subpools[0] = calloc(sizeof(tree_allocSubpool), 1);
    retPtr->subpools[0]->pStart = calloc(1, size),
    retPtr->subpools[0]->allocated = size;
    return retPtr;
}
void tree_allocDestroyPool(tree_allocPool pool){
    for(unsigned i = 0; i < pool->currentSubpool + 1; i++){
        free(pool->subpools[i]->pStart);
        free(pool->subpools[i]);
    }
    free(pool);
}

void* tree_allocate(tree_allocPool pool, size_t size){
    tree_allocSubpool* subpool = pool->subpools[pool->currentSubpool];
    if(!subpool->pStart) return (void*)0;
    if (subpool->allocated - subpool->used < size) {
        pool->currentSubpool++;
        pool->subpools[pool->currentSubpool] = calloc(subpool->allocated*2, 1);
        subpool = pool->subpools[pool->currentSubpool];
    }

    void* retptr = subpool->pStart + subpool->used;
    subpool->used += size;
    return retptr;
}
void tree_free(void* pointer){
    return;
};
