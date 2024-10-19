#include "tree_alloc.h"

tree_allocPool* tree_allocInitPool(size_t size){
    tree_allocPool* retPtr = calloc(sizeof(tree_allocPool), 1);
    *retPtr = (tree_allocPool){
        .pStart = malloc(size),
        .allocated = size,
    };
    return retPtr;
}
void tree_allocDestroyPool(tree_allocPool* pool){
    free(pool->pStart);
    free(pool);
}

void* tree_allocate(tree_allocPool* pool, size_t size){
    if(!pool->pStart) return (void*)0;
    if (pool->allocated - pool->used < size) {
        pool->pStart = realloc(pool->pStart, pool->allocated * 2);
        pool->allocated *= 2; 
    }

    void* retptr = pool->pStart + pool->used;
    pool->used += size;
    return retptr;
}
void tree_free(void* pointer){
    return;
};
