#include <stddef.h>
typedef struct tree_allocPool tree_allocPool;

struct tree_allocPool{
    void* pStart;
    size_t used;
    size_t allocated;
};

tree_allocPool* tree_allocInitPool(size_t size);
void tree_allocDestroyPool(tree_allocPool* pool);
void* tree_allocate(tree_allocPool* pool, size_t size);
void tree_free(void* pointer);